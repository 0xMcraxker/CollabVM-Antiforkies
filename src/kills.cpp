#include <windows.h>
#include <process.h>
#include <tlhelp32.h>
#include <winbase.h>
#include <string.h>

void killProcessByName(const char *filename)
{
    HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
    PROCESSENTRY32 pEntry;
    pEntry.dwSize = sizeof(pEntry);
    BOOL hRes = Process32First(hSnapShot, &pEntry);
    while (hRes)
    {
        // Convert szExeFile to lowercase
        char lowercaseFilename[MAX_PATH];
        strcpy_s(lowercaseFilename, MAX_PATH, pEntry.szExeFile);
        _strlwr_s(lowercaseFilename, MAX_PATH);

        if (strcmp(lowercaseFilename, filename) == 0)
        {
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0,
                                          (DWORD)pEntry.th32ProcessID);
            if (hProcess != NULL)
            {
                TerminateProcess(hProcess, 9);
                CloseHandle(hProcess);
            }
        }
        hRes = Process32Next(hSnapShot, &pEntry);
    }
    CloseHandle(hSnapShot);
}

void EnableShutdownPrivilege()
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tokenPrivileges;
    LUID luid;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return;

    if (!LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &luid))
    {
        CloseHandle(hToken);
        return;
    }

    tokenPrivileges.PrivilegeCount = 1;
    tokenPrivileges.Privileges[0].Luid = luid;
    tokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tokenPrivileges, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
    {
        CloseHandle(hToken);
        return;
    }

    CloseHandle(hToken);
}

void ShutdownSystem()
{
    EnableShutdownPrivilege();
    ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 0);
}
