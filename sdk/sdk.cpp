#include "sdk.hpp"

auto sdk::create_process_handle(char* path, _PROCESS_INFORMATION* pi) -> void
{
	_STARTUPINFOA si
	{ 
		sizeof(_STARTUPINFOA) 
	};

	CreateProcessA(NULL, path, NULL, NULL, FALSE, CREATE_SUSPENDED | CREATE_NEW_CONSOLE, NULL, NULL, &si, pi);;
}

auto sdk::find_remote_peb(void* handle) -> unsigned __int64
{
	static const auto proc_address = GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess");

	_PROCESS_BASIC_INFORMATION pbi;

	((long(_stdcall*)(void*, unsigned char, void*, unsigned long, unsigned long*))proc_address)
		(handle, 0, &pbi, sizeof(pbi), nullptr);

	return (unsigned __int64)pbi.PebBaseAddress;
}

auto sdk::patch_is_being_debugged(void* handle, unsigned __int64 peb) -> void
{
	auto buf = false;

	WriteProcessMemory(handle, (void*)(peb + 0x2), &buf, 1, nullptr);
}

auto sdk::patch_module_list(void* handle, unsigned __int64 peb, const char* module) -> void
{
	unsigned __int64 ldr = 0;

	ReadProcessMemory(handle, (void*)(peb + 0x18), &ldr, 8, nullptr);

	unsigned __int64 base = 0;

	ReadProcessMemory(handle, (void*)(ldr + 0x20), &base, 8, nullptr);

	for (unsigned __int64 link = base, last_link = 0; ;)
	{
		unsigned __int64 path = 0;

		ReadProcessMemory(handle, (void*)(link + 0x40), &path, 8, nullptr);

		if (path != 0)
		{
			std::string buf(260, 0);

			for (unsigned __int32 x = 0, s = buf.size(); x < s; ++x)
			{
				ReadProcessMemory(handle, (void*)(path + x * 2), &buf[x], 1, nullptr),
					buf[x] = std::tolower(buf[x]);

				if (std::isprint(buf[x]) == 0)
				{
					buf.resize(x), s = x;
				}
			}

			if (buf.find(module) != std::string::npos)
			{
				unsigned __int64 next_link = 0;

				ReadProcessMemory(handle, (void*)link, &next_link, 8, nullptr);

				WriteProcessMemory(handle, (void*)last_link, &next_link, 8, nullptr);

				break;
			}
		}

		last_link = link;

		ReadProcessMemory(handle, (void*)link, &link, 8, nullptr);

		if (link == base)
		{
			break;
		}
	}
}