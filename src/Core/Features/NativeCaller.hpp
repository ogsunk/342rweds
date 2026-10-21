#pragma once

#include <Includes/Includes.hpp>
#include <Core/SDK/SDK.hpp>

namespace Core::Features::NativeCaller
{
	inline uintptr_t g_pointer_to_handle = 0;
	inline uintptr_t g_get_native_handler = 0;
	inline uintptr_t g_fast_path_map = 0;
	inline uintptr_t g_rage_base = 0;
	inline uintptr_t g_rage_size = 0;
	inline bool g_map_pattern_found = false;
	inline bool g_pointer_pattern_found = false;
	inline std::atomic<bool> g_initialized{ false };
	inline std::atomic<bool> g_available{ false };
	inline std::atomic<int> g_init_attempts{ 0 };
	inline std::atomic<DWORD> g_resolved_pid{ 0 };
	inline bool g_handler_two_args = false;
	inline std::mutex g_init_mutex;

	inline uintptr_t FindModuleBase(const char* module_name)
	{
		HMODULE modules[1024]{};
		DWORD needed = 0;
		if (!EnumProcessModulesEx(Mem.ProcHandle, modules, sizeof(modules), &needed, LIST_MODULES_ALL))
		{
			if (!EnumProcessModules(Mem.ProcHandle, modules, sizeof(modules), &needed))
				return 0;
		}

		const size_t count = needed / sizeof(HMODULE);
		for (size_t i = 0; i < count; ++i)
		{
			char name[MAX_PATH]{};
			if (GetModuleBaseNameA(Mem.ProcHandle, modules[i], name, sizeof(name)) &&
				_stricmp(name, module_name) == 0)
			{
				return reinterpret_cast<uintptr_t>(modules[i]);
			}

			char path[MAX_PATH]{};
			if (GetModuleFileNameExA(Mem.ProcHandle, modules[i], path, sizeof(path)))
			{
				std::string full(path);
				const size_t pos = full.find_last_of("\\/");
				const std::string base = (pos == std::string::npos) ? full : full.substr(pos + 1);
				if (_stricmp(base.c_str(), module_name) == 0)
				{
					return reinterpret_cast<uintptr_t>(modules[i]);
				}
			}
		}

		return 0;
	}

	inline uint64_t FloatBits(float value)
	{
		uint64_t bits = 0;
		std::memcpy(&bits, &value, sizeof(float));
		return bits;
	}

	inline uint64_t RemoteCall1(uintptr_t func, uint64_t arg)
	{
		if (!func)
			return 0;

		constexpr size_t param_size = 0x20;
		const uintptr_t param = Mem.AllocateExecutableMemory(param_size);
		if (!param)
			return 0;

		Mem.Write<uint64_t>(param, func);
		Mem.Write<uint64_t>(param + 8, arg);
		Mem.Write<uint64_t>(param + 16, 0);

		std::vector<uint8_t> code;
		code.reserve(32);

		// mov rbx, <param>
		code.push_back(0x48);
		code.push_back(0xBB);
		for (int i = 0; i < 8; ++i)
			code.push_back(static_cast<uint8_t>((param >> (i * 8)) & 0xFF));

		code.insert(code.end(), { 0x48, 0x8B, 0x03 });       // mov rax, [rbx]
		code.insert(code.end(), { 0x48, 0x8B, 0x4B, 0x08 }); // mov rcx, [rbx+8]
		code.insert(code.end(), { 0xFF, 0xD0 });             // call rax
		code.insert(code.end(), { 0x48, 0x89, 0x43, 0x10 }); // mov [rbx+16], rax
		code.push_back(0xC3);                                // ret

		Mem.ExecuteShellcode(code.data(), code.size());
		const uint64_t result = Mem.Read<uint64_t>(param + 16);
		VirtualFreeEx(Mem.ProcHandle, reinterpret_cast<LPVOID>(param), 0, MEM_RELEASE);
		return result;
	}

	inline uint64_t RemoteCall2(uintptr_t func, uint64_t arg1, uint64_t arg2)
	{
		if (!func)
			return 0;

		constexpr size_t param_size = 0x28;
		const uintptr_t param = Mem.AllocateExecutableMemory(param_size);
		if (!param)
			return 0;

		Mem.Write<uint64_t>(param, func);
		Mem.Write<uint64_t>(param + 8, arg1);
		Mem.Write<uint64_t>(param + 16, arg2);
		Mem.Write<uint64_t>(param + 24, 0);

		std::vector<uint8_t> code;
		code.reserve(40);

		// mov rbx, <param>
		code.push_back(0x48);
		code.push_back(0xBB);
		for (int i = 0; i < 8; ++i)
			code.push_back(static_cast<uint8_t>((param >> (i * 8)) & 0xFF));

		code.insert(code.end(), { 0x48, 0x8B, 0x03 });       // mov rax, [rbx]
		code.insert(code.end(), { 0x48, 0x8B, 0x4B, 0x08 }); // mov rcx, [rbx+8]
		code.insert(code.end(), { 0x48, 0x8B, 0x53, 0x10 }); // mov rdx, [rbx+16]
		code.insert(code.end(), { 0xFF, 0xD0 });             // call rax
		code.insert(code.end(), { 0x48, 0x89, 0x43, 0x18 }); // mov [rbx+24], rax
		code.push_back(0xC3);                                // ret

		Mem.ExecuteShellcode(code.data(), code.size());
		const uint64_t result = Mem.Read<uint64_t>(param + 24);
		VirtualFreeEx(Mem.ProcHandle, reinterpret_cast<LPVOID>(param), 0, MEM_RELEASE);
		return result;
	}

	inline bool FindModuleInfo(const char* module_name, uintptr_t& out_base, uintptr_t& out_size)
	{
		out_base = 0;
		out_size = 0;

		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, Mem.ProcId);
		if (snapshot == INVALID_HANDLE_VALUE)
			return false;

		MODULEENTRY32 entry{};
		entry.dwSize = sizeof(entry);

		bool found = false;
		if (Module32First(snapshot, &entry))
		{
			do
			{
				if (_stricmp(entry.szModule, module_name) == 0)
				{
					out_base = reinterpret_cast<uintptr_t>(entry.modBaseAddr);
					out_size = static_cast<uintptr_t>(entry.modBaseSize);
					found = true;
					break;
				}
			} while (Module32Next(snapshot, &entry));
		}

		CloseHandle(snapshot);
		return found;
	}

	inline uintptr_t GetModuleSizeFromPE(uintptr_t module_base)
	{
		if (!module_base)
			return 0;

		const uint32_t e_lfanew = Mem.Read<uint32_t>(module_base + 0x3C);
		if (!e_lfanew)
			return 0;

		const uintptr_t nt = module_base + e_lfanew;
		if (Mem.Read<uint32_t>(nt) != 0x00004550) // 'PE\0\0'
			return 0;

		return Mem.Read<uint32_t>(nt + 0x18 + 0x38); // OptionalHeader.SizeOfImage
	}

	inline uintptr_t FindPatternInRange(const char* pattern, uintptr_t base, size_t size)
	{
		if (!pattern || !base || !size)
			return 0;

		std::vector<uint8_t> signature;
		std::vector<bool> mask;
		std::stringstream stream(pattern);
		std::string token;
		while (stream >> token)
		{
			if (token == "?" || token == "??")
			{
				signature.push_back(0);
				mask.push_back(false);
			}
			else
			{
				signature.push_back(static_cast<uint8_t>(std::stoul(token, nullptr, 16)));
				mask.push_back(true);
			}
		}

		if (signature.empty())
			return 0;

		uintptr_t address = base;
		while (address < base + size)
		{
			MEMORY_BASIC_INFORMATION mbi{};
			if (!VirtualQueryEx(Mem.ProcHandle, reinterpret_cast<LPCVOID>(address), &mbi, sizeof(mbi)))
				break;

			const uintptr_t region_base = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
			const size_t region_size = mbi.RegionSize;

			const bool readable = mbi.State == MEM_COMMIT &&
				(mbi.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY));

			if (readable && region_size >= signature.size())
			{
				std::vector<uint8_t> buffer(region_size);
				SIZE_T bytes_read = 0;
				if (ReadProcessMemory(Mem.ProcHandle, reinterpret_cast<LPCVOID>(region_base), buffer.data(), region_size, &bytes_read) &&
					bytes_read >= signature.size())
				{
					for (size_t i = 0; i + signature.size() <= bytes_read; ++i)
					{
						bool found = true;
						for (size_t j = 0; j < signature.size(); ++j)
						{
							if (mask[j] && buffer[i + j] != signature[j])
							{
								found = false;
								break;
							}
						}

						if (found)
							return region_base + i;
					}
				}
			}

			if (region_size == 0)
				break;

			address = region_base + region_size;
		}

		return 0;
	}

	inline uintptr_t ResolveExport(uintptr_t module_base, const char* name_part)
	{
		if (!module_base || !name_part)
			return 0;

		const uint32_t e_lfanew = Mem.Read<uint32_t>(module_base + 0x3C);
		if (!e_lfanew)
			return 0;

		const uintptr_t nt = module_base + e_lfanew;
		if (Mem.Read<uint32_t>(nt) != 0x00004550) // 'PE\0\0'
			return 0;

		const uintptr_t optional = nt + 0x18;
		const uint16_t magic = Mem.Read<uint16_t>(optional);
		const uintptr_t data_dir = optional + (magic == 0x20B ? 0x70 : 0x60);

		const uint32_t export_rva = Mem.Read<uint32_t>(data_dir + 0x00);
		if (!export_rva)
			return 0;

		const uintptr_t export_dir = module_base + export_rva;
		const uint32_t num_funcs = Mem.Read<uint32_t>(export_dir + 0x14);
		const uint32_t num_names = Mem.Read<uint32_t>(export_dir + 0x18);
		const uint32_t funcs_rva = Mem.Read<uint32_t>(export_dir + 0x1C);
		const uint32_t names_rva = Mem.Read<uint32_t>(export_dir + 0x20);
		const uint32_t ords_rva = Mem.Read<uint32_t>(export_dir + 0x24);

		if (!num_funcs || !num_names || !funcs_rva || !names_rva || !ords_rva)
			return 0;

		for (uint32_t i = 0; i < num_names; ++i)
		{
			const uint32_t name_offset = Mem.Read<uint32_t>(module_base + names_rva + i * 4);
			if (!name_offset)
				continue;

			const std::string export_name = Mem.ReadString(module_base + name_offset);
			if (export_name.find(name_part) == std::string::npos)
				continue;

			const uint16_t ordinal = Mem.Read<uint16_t>(module_base + ords_rva + i * 2);
			if (ordinal >= num_funcs)
				continue;

			const uint32_t func_rva = Mem.Read<uint32_t>(module_base + funcs_rva + ordinal * 4);
			if (!func_rva)
				continue;

			return module_base + func_rva;
		}

		return 0;
	}

	inline bool Initialize()
	{
		std::lock_guard<std::mutex> lock(g_init_mutex);

		// Already fully resolved for the process we are currently attached to.
		if (g_available.load() && g_rage_base && g_get_native_handler &&
			g_pointer_to_handle && g_resolved_pid.load() == Mem.ProcId)
			return true;

		// Never attempt resolution until the target is fully attached. A
		// transient detach (ModBase/ProcId == 0 during a re-attach) must not be
		// allowed to corrupt an otherwise good resolution.
		if (!Mem.ProcHandle || !Mem.ProcId || !Mem.ModBase || !Mem.ModBaseSize)
			return false;

		g_init_attempts.fetch_add(1);

		// Resolve everything into locals first. Globals are only mutated once a
		// piece is known-good, so a failed attempt can never wipe out a previous
		// success.
		uintptr_t rage_base = 0;
		uintptr_t rage_size = 0;
		if (!FindModuleInfo("rage-scripting-five.dll", rage_base, rage_size))
		{
			rage_base = FindModuleBase("rage-scripting-five.dll");
		}
		if (!rage_base)
			rage_base = Mem.ModBase;
		if (!rage_size)
			rage_size = GetModuleSizeFromPE(rage_base);
		if (!rage_size && rage_base == Mem.ModBase)
			rage_size = Mem.ModBaseSize;
		if (!rage_base || !rage_size)
			return false;

		// rage-scripting-five.dll exports
		//   scrEngine::GetNativeHandler(uint64_t hash) -> NativeHandler
		// (RVA 0x16E60 on the b3258 client). Fall back to the internal
		// resolver if the export is unavailable.
		uintptr_t handler = ResolveExport(rage_base, "GetNativeHandler");
		bool handler_two_args = false;
		if (!handler)
		{
			handler = Mem.FindSignatureStr("4C 8B DC 55 56 57 41 57", rage_base, rage_size);
			if (handler)
			{
				handler_two_args = true; // (hash, mappedNative)
			}
			else
			{
				handler = Mem.FindSignatureStr("40 55 56 57 48 83 EC", rage_base, rage_size);
			}
		}

		// pointer_to_handle is a game function in the main module. Prefer the
		// known per-build RVA, then fall back to signatures.
		uintptr_t pointer_to_handle = 0;
		switch (g_Offsets.CurrentBuild)
		{
		case 3258: pointer_to_handle = Mem.ModBase + 0x1639CE8; break;
		case 3095: pointer_to_handle = Mem.ModBase + 0x1624F80; break;
		case 2944: pointer_to_handle = Mem.ModBase + 0x160FF90; break;
		case 2802: pointer_to_handle = Mem.ModBase + 0x15EA290; break;
		case 2699: pointer_to_handle = Mem.ModBase + 0x15D6CEC; break;
		case 2612: pointer_to_handle = Mem.ModBase + 0x15DFE20; break;
		case 2545: pointer_to_handle = Mem.ModBase + 0x15DEEB0; break;
		case 2372: pointer_to_handle = Mem.ModBase + 0x15CD2C0; break;
		case 2189: pointer_to_handle = Mem.ModBase + 0x15B1008; break;
		case 2060: pointer_to_handle = Mem.ModBase + 0x159B224; break;
		default: pointer_to_handle = 0; break;
		}

		if (!pointer_to_handle)
		{
			const char* long_sig = "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 8B 15 ? ? ? ? 48 8B F9 48 83 C1 10 33 DB";
			const char* short_sig = "48 8B F9 48 83 C1 10 33 DB";

			uintptr_t found = Mem.FindSignatureStr(long_sig, Mem.ModBase, Mem.ModBaseSize);
			if (!found)
				found = FindPatternInRange(long_sig, Mem.ModBase, Mem.ModBaseSize);
			if (found)
			{
				pointer_to_handle = found;
			}
			else
			{
				found = Mem.FindSignatureStr(short_sig, Mem.ModBase, Mem.ModBaseSize);
				if (!found)
					found = FindPatternInRange(short_sig, Mem.ModBase, Mem.ModBaseSize);
				if (found)
					pointer_to_handle = found - 0x15;
			}
		}

		// Commit. Only overwrite a global when we actually resolved a value.
		g_rage_base = rage_base;
		g_rage_size = rage_size;
		if (handler)
			g_get_native_handler = handler;
		if (pointer_to_handle)
			g_pointer_to_handle = pointer_to_handle;
		g_handler_two_args = handler_two_args;
		g_map_pattern_found = g_get_native_handler != 0;
		g_pointer_pattern_found = g_pointer_to_handle != 0;

		const bool available = g_get_native_handler != 0 && g_pointer_to_handle != 0;
		g_available.store(available);
		if (available)
			g_resolved_pid.store(Mem.ProcId);
		return available;
	}

	inline bool IsAvailable()
	{
		return Initialize();
	}

	inline std::string Diagnostics()
	{
		Initialize();
		return std::string("r=") + (g_rage_base ? "1" : "0") +
			" m=" + (g_map_pattern_found ? "1" : "0") +
			" p=" + (g_pointer_pattern_found ? "1" : "0") +
			" b=" + (Mem.ModBase ? "1" : "0") +
			" h=" + (Mem.ProcHandle ? "1" : "0") +
			" s=" + (Mem.ModBaseSize ? "1" : "0") +
			" rs=" + (g_rage_size ? "1" : "0") +
			" a=" + std::to_string(g_init_attempts.load()) +
			" build=" + std::to_string(g_Offsets.CurrentBuild);
	}

	inline uint32_t PointerToHandle(uintptr_t entity)
	{
		if (!Initialize())
			return 0;

		return static_cast<uint32_t>(RemoteCall1(g_pointer_to_handle, entity));
	}

	inline uintptr_t GetNativeHandler(uint64_t hash)
	{
		if (!Initialize() || !g_get_native_handler)
			return 0;

		if (g_handler_two_args)
			return static_cast<uintptr_t>(RemoteCall2(g_get_native_handler, hash, hash));

		return static_cast<uintptr_t>(RemoteCall1(g_get_native_handler, hash));
	}

	inline bool InvokeNative(uint64_t hash, const std::vector<uint64_t>& args, uint64_t* result = nullptr)
	{
		if (!Initialize())
			return false;

		const uintptr_t handler = GetNativeHandler(hash);
		if (!handler)
			return false;

		constexpr size_t context_size = 0x100;
		constexpr size_t return_size = 0x10;
		constexpr size_t args_size = 0x80;
		const size_t total_size = context_size + return_size + args_size;

		const uintptr_t block = Mem.AllocateExecutableMemory(total_size);
		if (!block)
			return false;

		const uintptr_t return_ptr = block + context_size;
		const uintptr_t args_ptr = return_ptr + return_size;

		for (size_t i = 0; i < total_size; i += 8)
			Mem.Write<uint64_t>(block + i, 0);

		Mem.Write<uint64_t>(block + 0x00, return_ptr);
		Mem.Write<uint32_t>(block + 0x08, static_cast<uint32_t>(args.size()));
		Mem.Write<uint64_t>(block + 0x10, args_ptr);
		Mem.Write<uint32_t>(block + 0x18, 0);

		for (size_t i = 0; i < args.size() && i < 16; ++i)
			Mem.Write<uint64_t>(args_ptr + i * 8, args[i]);

		RemoteCall1(handler, block);

		if (result)
			*result = Mem.Read<uint64_t>(return_ptr);

		VirtualFreeEx(Mem.ProcHandle, reinterpret_cast<LPVOID>(block), 0, MEM_RELEASE);
		return true;
	}
}
