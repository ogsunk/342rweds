#pragma once

#include <Includes/Includes.hpp>
#include <Core/SDK/SDK.hpp>
#include <Core/Features/NativeCaller.hpp>

// Time / weather overrides done with pure memory writes.
//
// SET_CLOCK_TIME / SET_WEATHER_TYPE_NOW touch the game's script VM and crash
// when called from an external thread with no script context, so the globals
// behind them are located by signature and written directly instead.
namespace Core::Features::World
{
	inline uintptr_t g_clock_address = 0;
	inline bool g_clock_is_pointer = false;
	inline uintptr_t g_weather_address = 0;
	inline uintptr_t g_weather_next_address = 0;
	inline bool g_scan_attempted = false;
	inline std::atomic<bool> g_scan_started{ false };
	inline std::atomic<bool> g_scan_done{ false };
	inline bool g_weather_verified = false;

	inline bool IsKnownWeatherHash(uint32_t value)
	{
		static const uint32_t known[] = {
			0x97AA0A79, // EXTRASUNNY
			0x36A83D84, // CLEAR
			0x30FDAF5C, // CLOUDS
			0xBB898D2D, // OVERCAST
			0x54A69840, // RAIN
			0x6DB1A50D, // CLEARING
			0xB677829F, // THUNDER
			0x10DCF4B5, // SMOG
			0xAE737644, // FOGGY
			0xAAC9C895, // XMAS
			0xEFB6EFF6, // SNOW
			0x27EA2814  // BLIZZARD
		};

		for (const uint32_t hash : known)
		{
			if (hash == value)
				return true;
		}

		return false;
	}

	inline uintptr_t ResolveRip(uintptr_t instruction, int displacement_offset, int instruction_length)
	{
		if (!instruction)
			return 0;

		const int32_t displacement = Mem.Read<int32_t>(instruction + displacement_offset);
		return instruction + instruction_length + displacement;
	}

	inline void ScanAddressesNow()
	{
		if (g_scan_attempted)
			return;

		g_scan_attempted = true;

		if (!Mem.ModBase || !Mem.ModBaseSize)
		{
			g_scan_attempted = false;
			return;
		}

		// Weather: lea rdx, [rip+x] / jmp / lea rdx, [rip+y]
		{
			const uintptr_t instruction = Mem.FindSignatureStr(
				"48 8D 15 ?? ?? ?? ?? EB ?? 48 8D 15", Mem.ModBase, Mem.ModBaseSize);

			if (instruction)
			{
				const uintptr_t first = ResolveRip(instruction, 3, 7);
				const uintptr_t second = ResolveRip(instruction + 9, 3, 7);

				if (NativeCaller::IsReadable(first, sizeof(uint32_t)))
					g_weather_address = first;

				if (NativeCaller::IsReadable(second, sizeof(uint32_t)))
					g_weather_next_address = second;
			}

			if (!g_weather_address)
			{
				const uintptr_t simpler = Mem.FindSignatureStr(
					"83 3D ?? ?? ?? ?? 00 74 ?? E8", Mem.ModBase, Mem.ModBaseSize);

				if (simpler)
				{
					const uintptr_t target = ResolveRip(simpler, 2, 6);
					if (NativeCaller::IsReadable(target, sizeof(uint32_t)))
						g_weather_address = target;
				}
			}

			if (!g_weather_address)
			{
				const uintptr_t fallback = Mem.FindSignatureStr(
					"8B 05 ?? ?? ?? ?? 89 05", Mem.ModBase, Mem.ModBaseSize);

				if (fallback)
				{
					const uintptr_t first = ResolveRip(fallback, 2, 6);
					const uintptr_t second = ResolveRip(fallback + 6, 2, 6);

					if (NativeCaller::IsReadable(first, sizeof(uint32_t)))
						g_weather_address = first;

					if (NativeCaller::IsReadable(second, sizeof(uint32_t)))
						g_weather_next_address = second;
				}
			}
		}

		// A resolved weather address is only trusted when it already holds a
		// real weather hash; that rules out a stray signature match.
		if (g_weather_address &&
			NativeCaller::IsReadable(g_weather_address, sizeof(uint32_t)) &&
			IsKnownWeatherHash(Mem.Read<uint32_t>(g_weather_address)))
		{
			g_weather_verified = true;
		}

		// Clock: a global struct with hour/minute/second at +0x10/+0x14/+0x18.
		{
			const uintptr_t direct = Mem.FindSignatureStr(
				"48 8D 0D ?? ?? ?? ?? BA ?? ?? ?? ?? E8 ?? ?? ?? ?? 44 8B 05",
				Mem.ModBase, Mem.ModBaseSize);

			if (direct)
			{
				g_clock_address = ResolveRip(direct, 3, 7);
				g_clock_is_pointer = false;
			}
			else
			{
				uintptr_t pointer_instruction = Mem.FindSignatureStr(
					"48 8B 15 ?? ?? ?? ?? 48 85 D2 74", Mem.ModBase, Mem.ModBaseSize);

				if (!pointer_instruction)
					pointer_instruction = Mem.FindSignatureStr(
						"48 8B 05 ?? ?? ?? ?? 0F B6 48 10", Mem.ModBase, Mem.ModBaseSize);

				if (!pointer_instruction)
					pointer_instruction = Mem.FindSignatureStr(
						"48 8B 0D ?? ?? ?? ?? 8B 41 10", Mem.ModBase, Mem.ModBaseSize);

				if (pointer_instruction)
				{
					g_clock_address = ResolveRip(pointer_instruction, 3, 7);
					g_clock_is_pointer = true;
				}
			}
		}
	}

	// The signature scan walks the whole game module, so it runs once on a
	// background thread instead of stalling the render thread.
	inline void EnsureScanStarted()
	{
		bool expected = false;
		if (!g_scan_started.compare_exchange_strong(expected, true))
			return;

		std::thread([] {
			ScanAddressesNow();
			g_scan_done.store(true);
		}).detach();
	}

	inline bool WeatherAvailable()
	{
		EnsureScanStarted();
		if (!g_scan_done.load())
			return false;

		return g_weather_address != 0;
	}

	inline bool TimeAvailable()
	{
		EnsureScanStarted();
		if (!g_scan_done.load())
			return false;

		if (!g_clock_address)
			return false;

		const uintptr_t base = g_clock_is_pointer
			? Mem.Read<uintptr_t>(g_clock_address)
			: g_clock_address;

		return base && NativeCaller::IsReadable(base + 0x10, 12);
	}

	inline bool ApplyWeather(uint32_t weather_hash)
	{
		EnsureScanStarted();
		if (!g_scan_done.load())
			return false;

		if (!g_weather_address || !weather_hash)
			return false;

		if (!NativeCaller::IsReadable(g_weather_address, sizeof(uint32_t)))
			return false;

		Mem.Write<uint32_t>(g_weather_address, weather_hash);

		if (g_weather_next_address &&
			NativeCaller::IsReadable(g_weather_next_address, sizeof(uint32_t)))
		{
			Mem.Write<uint32_t>(g_weather_next_address, weather_hash);
		}

		return true;
	}

	inline bool ApplyClock(int hour, int minute)
	{
		EnsureScanStarted();
		if (!g_scan_done.load())
			return false;

		if (!g_clock_address)
			return false;

		uintptr_t base = g_clock_address;
		if (g_clock_is_pointer)
		{
			base = Mem.Read<uintptr_t>(g_clock_address);
			if (!base || base > 0x7FFFFFFFFFFF)
				return false;
		}

		if (!NativeCaller::IsReadable(base + 0x10, 12))
			return false;

		// Validate before writing: a wrong address must never be clobbered.
		const int32_t current_hour = Mem.Read<int32_t>(base + 0x10);
		const int32_t current_minute = Mem.Read<int32_t>(base + 0x14);

		if (current_hour < 0 || current_hour > 23 || current_minute < 0 || current_minute > 59)
			return false;

		Mem.Write<int32_t>(base + 0x10, std::clamp(hour, 0, 23));
		Mem.Write<int32_t>(base + 0x14, std::clamp(minute, 0, 59));
		Mem.Write<int32_t>(base + 0x18, 0);
		return true;
	}
}
