#include <psapi.h>
#include <winbase.h>
#include <windows.h>

void killProcessByName(const char *filename);
void EnableShutdownPrivilege();
void ShutdownSystem();