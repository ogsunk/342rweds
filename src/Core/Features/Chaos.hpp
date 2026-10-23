#pragma once

#include <Includes/Includes.hpp>
#include <Core/SDK/SDK.hpp>
#include <Core/Features/NativeCaller.hpp>

// Shared helpers for the Misc > Chaos tab.
//
// Everything here is defensive on purpose: entities stream in and out constantly
// in FiveM, so a pointer that was valid when the entity list was built can be
// freed a few milliseconds later. Handing a stale pointer to PointerToHandle
// makes the game resolve a bogus handle and crashes it, so every entity access
// is validated first and every action is serialized.
namespace Core::Features::Chaos
{
	inline std::atomic<bool> g_spawn_busy{ false };
	inline std::atomic<int> g_spawn_kind{ 0 }; // 0 = idle, 1 = cars, 2 = peds

	inline uint32_t Joaat(const char* text)
	{
		uint32_t hash = 0;
		if (!text)
			return 0;

		while (*text)
		{
			hash += static_cast<uint8_t>(std::tolower(static_cast<unsigned char>(*text++)));
			hash += hash << 10;
			hash ^= hash >> 6;
		}

		hash += hash << 3;
		hash ^= hash >> 11;
		hash += hash << 15;
		return hash;
	}

	inline bool PositionIsSane(const D3DXVECTOR3& position)
	{
		return std::isfinite(position.x) && std::isfinite(position.y) && std::isfinite(position.z) &&
			std::fabs(position.x) < 20000.0f && std::fabs(position.y) < 20000.0f &&
			std::fabs(position.z) < 5000.0f;
	}

	inline bool PointerIsUsable(uintptr_t address)
	{
		return address != 0 && NativeCaller::IsReadable(address, 0x100);
	}

	// A ped/vehicle pointer is only safe to use when the memory is still mapped
	// and its position still looks like a real world coordinate.
	inline bool EntityIsUsable(uintptr_t address)
	{
		if (!PointerIsUsable(address))
			return false;

		const D3DXVECTOR3 position = Mem.Read<D3DXVECTOR3>(address + 0x90);
		return PositionIsSane(position);
	}

	inline bool VehicleIsUsable(uintptr_t address)
	{
		if (!EntityIsUsable(address))
			return false;

		if (g_Offsets.m_VehicleEngineHealth &&
			!NativeCaller::IsReadable(address + g_Offsets.m_VehicleEngineHealth, sizeof(float)))
		{
			return false;
		}

		return true;
	}

	inline int LocalPlayerIndex()
	{
		static std::atomic<int> cached{ -1 };
		static std::atomic<ULONGLONG> stamp{ 0 };

		const ULONGLONG now = GetTickCount64();
		if (cached.load() < 0 || now - stamp.load() > 5000)
		{
			uint64_t result = 0;
			if (NativeCaller::InvokeNative(0x4F8644AF03D0E0D6ull, {}, &result))
			{
				cached.store(static_cast<int>(result));
				stamp.store(now);
			}
		}

		const int index = cached.load();
		return index < 0 ? 0 : index;
	}

	// ------------------------------------------------------------------
	// Spawning is frame driven on purpose. Creating entities from a worker
	// thread races with the game's streaming and the render thread's own
	// native calls, which is what was crashing the game.
	// ------------------------------------------------------------------
	struct SpawnJob
	{
		int kind = 0;
		uint32_t model = 0;
		int remaining = 0;
		D3DXVECTOR3 origin{};
		float radius = 12.0f;
		ULONGLONG requested_at = 0;
		ULONGLONG next_spawn_at = 0;
		bool model_ready = false;
	};

	inline SpawnJob g_job;
	inline std::mutex g_job_mutex;

	inline void CancelSpawnLocked()
	{
		if (g_job.model)
			NativeCaller::InvokeNative(0xE532F5D78798DAABull, { g_job.model }); // SET_MODEL_AS_NO_LONGER_NEEDED

		g_job = SpawnJob{};
		g_spawn_kind.store(0);
		g_spawn_busy.store(false);
	}

	inline bool StartSpawn(int kind, uint32_t model, int count, const D3DXVECTOR3& origin, float radius)
	{
		if ((kind != 1 && kind != 2) || model == 0 || !NativeCaller::IsAvailable())
			return false;

		bool expected = false;
		if (!g_spawn_busy.compare_exchange_strong(expected, true))
			return false;

		{
			std::lock_guard<std::mutex> lock(g_job_mutex);

			g_job = SpawnJob{};
			g_job.kind = kind;
			g_job.model = model;
			g_job.remaining = std::clamp(count, 1, 30);
			g_job.origin = origin;
			g_job.radius = std::clamp(radius, 6.0f, 40.0f);
		}

		g_spawn_kind.store(kind);
		return true;
	}

	inline void Tick()
	{
		if (g_spawn_kind.load() == 0)
			return;

		std::lock_guard<std::mutex> lock(g_job_mutex);

		if (g_job.kind == 0)
			return;

		if (!NativeCaller::IsAvailable())
		{
			CancelSpawnLocked();
			return;
		}

		const ULONGLONG now = GetTickCount64();

		if (!g_job.model_ready)
		{
			if (g_job.requested_at == 0)
			{
				NativeCaller::InvokeNative(0x963D27A58DF860ACull, { g_job.model }); // REQUEST_MODEL
				g_job.requested_at = now;
			}

			if (now - g_job.requested_at > 8000)
			{
				CancelSpawnLocked();
				return;
			}

			uint64_t loaded = 0;
			if (now - g_job.requested_at > 80 &&
				NativeCaller::InvokeNative(0x98A4EB5D89A0C952ull, { g_job.model }, &loaded) && loaded)
			{
				g_job.model_ready = true;
			}

			return;
		}

		if (now < g_job.next_spawn_at)
			return;

		static std::mt19937 rng(std::random_device{}());
		std::uniform_real_distribution<float> angle_dist(0.0f, 6.2831853f);
		std::uniform_real_distribution<float> radius_dist(5.0f, (std::max)(g_job.radius, 6.0f));

		for (int i = 0; i < 2 && g_job.remaining > 0; ++i, --g_job.remaining)
		{
			const float angle = angle_dist(rng);
			const float distance = radius_dist(rng);
			const float x = g_job.origin.x + std::cos(angle) * distance;
			const float y = g_job.origin.y + std::sin(angle) * distance;
			const float z = g_job.origin.z + 0.5f;
			const float heading = angle_dist(rng) * 57.29578f;

			if (g_job.kind == 1)
			{
				uint64_t vehicle = 0;
				NativeCaller::InvokeNative(
					0xAF35D0D2583051B0ull, // CREATE_VEHICLE
					{
						g_job.model,
						NativeCaller::FloatBits(x),
						NativeCaller::FloatBits(y),
						NativeCaller::FloatBits(z),
						NativeCaller::FloatBits(heading),
						1ull,
						0ull
					},
					&vehicle);

				if (vehicle)
				{
					// Only touch the returned handle, never a cached pointer.
					Core::Features::NativeCaller::InvokeNative(0x49733E92263139D1ull, { vehicle });
				}
			}
			else
			{
				NativeCaller::InvokeNative(
					0xD49F9B0955C367DEull, // CREATE_PED
					{
						4ull, // civilian male
						g_job.model,
						NativeCaller::FloatBits(x),
						NativeCaller::FloatBits(y),
						NativeCaller::FloatBits(z),
						NativeCaller::FloatBits(heading),
						1ull,
						0ull
					});
			}
		}

		g_job.next_spawn_at = now + 80;

		if (g_job.remaining <= 0)
			CancelSpawnLocked();
	}

	inline bool SpawnBusy()
	{
		return g_spawn_kind.load() != 0;
	}
}
