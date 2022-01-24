#include "sdk/sdk.hpp"

int main(__int32 argc, char* argv[])
{
    _PROCESS_INFORMATION pi;
    sdk::create_process_handle(argv[1], &pi);

    if (pi.hProcess != nullptr)
    {
        auto peb = sdk::find_remote_peb(pi.hProcess);

        for (unsigned __int32 x = 0;; Sleep(10), ++x)
        {
            sdk::patch_is_being_debugged(pi.hProcess, peb);

            sdk::patch_module_list(pi.hProcess, peb, "binary.dll");

            /* just delay this shit lol */

            if (x == 100) sdk::inject(pi.hProcess, "binary.dll");

            if (x == 300) ResumeThread(pi.hThread);
        }
    }

    return std::cin.get() != 0;
}