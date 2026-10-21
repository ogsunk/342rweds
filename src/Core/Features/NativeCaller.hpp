#pragma once

#include <Includes/Includes.hpp>
#include <Core/SDK/SDK.hpp>

namespace Core::Features::NativeCaller
{
	inline uintptr_t g_pointer_to_handle = 0;
	inline uintptr_t g_get_native_handler = 0;
	inline std::atomic<bool> g_initialized{ false };
	inline std::atomic<bool> g_available{ false };
	inline std::mutex g_init_mutex;

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

	inline bool Initialize()
	{
		std::lock_guard<std::mutex> lock(g_init_mutex);

		if (g_initialized.load())
			return g_available.load();

		g_initialized.store(true);

		uintptr_t rage_size = 0;
		const uintptr_t rage_base = Mem.GetModuleBaseAddr(Mem.ProcId, "rage-scripting-five.dll", &rage_size);
		if (!rage_base)
		{
			g_available.store(false);
			return false;
		}

		g_get_native_handler = Mem.FindSignatureStr("4C 8B DC 55 56 57 41 57", rage_base, rage_size);
		if (!g_get_native_handler)
		{
			g_get_native_handler = Mem.FindSignatureStr("40 55 56 57 48 83 EC", rage_base, rage_size);
		}

		g_pointer_to_handle = Mem.FindSignatureStr(
			"48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 8B 15 ? ? ? ? 48 8B F9 48 83 C1 10 33 DB",
			Mem.ModBase,
			Mem.ModBaseSize);

		g_available.store(g_get_native_handler != 0 && g_pointer_to_handle != 0);
		return g_available.load();
	}

	inline bool IsAvailable()
	{
		return Initialize();
	}

	inline uint32_t PointerToHandle(uintptr_t entity)
	{
		if (!Initialize())
			return 0;

		return static_cast<uint32_t>(RemoteCall1(g_pointer_to_handle, entity));
	}

	inline uintptr_t GetNativeHandler(uint64_t hash)
	{
		if (!Initialize())
			return 0;

		return static_cast<uintptr_t>(RemoteCall2(g_get_native_handler, hash, hash));
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
