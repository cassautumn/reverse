#include <windows.h>
#include <winternl.h>
#include <tlhelp32.h>
#include <iostream>
#include <vector>
#include <intrin.h>

namespace sdk
{
	extern auto create_process_handle(char* path, _PROCESS_INFORMATION* pi) -> void;

	extern auto find_remote_peb(void* handle) -> unsigned __int64;

	extern auto patch_is_being_debugged(void* handle, unsigned __int64 peb) -> void;

	extern auto patch_module_list(void* handle, unsigned __int64 peb, const char* module) -> void;
}

namespace sdk
{
	extern __forceinline auto inject(void* handle, const char* name) -> void
	{
		std::string path(255, 0);

		GetFullPathNameA(name, 255, &path[0], 0);

		auto remote_memory = VirtualAllocEx(handle, 0x0, path.size() + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

		WriteProcessMemory(handle, remote_memory, path.data(), path.size(), 0);

		static const auto load_lib = GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");

		auto thread = CreateRemoteThread(handle, nullptr, 0, (LPTHREAD_START_ROUTINE)load_lib, remote_memory, 0, nullptr);

		WaitForSingleObject(thread, INFINITE), CloseHandle(thread);

		VirtualFreeEx(handle, remote_memory, path.size() + 1, MEM_FREE);
	}
}