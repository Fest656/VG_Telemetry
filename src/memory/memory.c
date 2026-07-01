#include "memory.h"
#include <tlhelp32.h> 
#include <stdio.h>    
#include <string.h>   
#include <Windows.h>

/*
https://learn.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-createtoolhelp32snapshot
https://learn.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-process32first
https://learn.microsoft.com/en-us/windows/win32/api/tlhelp32/ns-tlhelp32-processentry32
*/
int memGetProcessId(const char *processName, DWORD *pidPtr) {
    HANDLE handleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 
    if (handleSnapshot == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        printf("Error: memGetProcessId could not create process snapshot. Win32 Error: %lu\n", error);
        return 0;
    }
    
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    BOOL hasNext = Process32First(handleSnapshot, &entry);
    if (hasNext == FALSE) {
        DWORD error = GetLastError();
        printf("Error: memGetProcessId could not get first process entry. Win32 Error: %lu\n", error);
        CloseHandle(handleSnapshot);
        return 0;
    }

    while (hasNext == TRUE) {
        if (strcmp(entry.szExeFile, processName) == 0) {
            *pidPtr = entry.th32ProcessID;
            CloseHandle(handleSnapshot);
            return 1;
        }
        hasNext = Process32Next(handleSnapshot, &entry);
    }

    printf("Error: Process was not found.\n");
    CloseHandle(handleSnapshot);
    return 0;
}   

/*
https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-openprocess
https://learn.microsoft.com/en-us/windows/win32/procthread/process-security-and-access-rights
https://learn.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-closehandle
*/
int memOpenProcess(DWORD pid, HANDLE *handlePtr) {
    *handlePtr = OpenProcess(PROCESS_VM_READ, FALSE, pid);
    if (*handlePtr == NULL) {
        DWORD error = GetLastError();
        printf("Error: memOpenProcess could not open process with PID %lu. Win32 Error: %lu\n", pid, error);
        return 0;
    }
    return 1;
}

/*
Very similar to getting the PID, the difference being the keywords used in CreateToolhelp32Snapshot
We use TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32 because this process is 64 bit and assault cube is a 32 bit application
*/
int memGetModuleBase(DWORD pid, const char *moduleName, uintptr_t *baseAddressPtr) {
    HANDLE handleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid); 
    if (handleSnapshot == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        printf("Error: memGetModuleBase could not create module snapshot for PID %lu. Win32 Error: %lu\n", pid, error);
        return 0;
    }

    MODULEENTRY32 entry;
    entry.dwSize = sizeof(MODULEENTRY32);
    BOOL hasNext = Module32First(handleSnapshot, &entry);
    if (hasNext == FALSE) {
        DWORD error = GetLastError();
        printf("Error: memGetModuleBase could not get first module entry. Win32 Error: %lu\n", error);
        CloseHandle(handleSnapshot);
        return 0;
    }

    while (hasNext == TRUE) {
        if (strcmp(entry.szModule, moduleName) == 0) {
            *baseAddressPtr = (uintptr_t)entry.modBaseAddr;
            CloseHandle(handleSnapshot);
            return 1;
        }
        hasNext = Module32Next(handleSnapshot, &entry);
    }

    printf("Error: Module was not found.\n");
    CloseHandle(handleSnapshot);
    return 0;
}

/*
https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-readprocessmemory
*/
int memReadInt(HANDLE processHandle, uintptr_t address, int *valuePtr) {
    SIZE_T bytesToRead = sizeof(int);
    SIZE_T bytesRead = 0;
    LPCVOID baseAddress = (LPCVOID)address;
    LPVOID lpBuffer = (LPVOID)valuePtr;

    if (ReadProcessMemory(processHandle, baseAddress, lpBuffer, bytesToRead, &bytesRead) == 0) {
        DWORD error = GetLastError();
        printf("Error: memReadInt failed to read at address 0x%08X. Bytes read: %zu. Win32 Error: %lu\n",
               (unsigned int)address, bytesRead, error);
        return 0;
    }
    return 1;
}

/*
Same as memReadInt except we use this to read a uintptr instead of a simple int type, this is better for pointer arithmetic and is simpler to follow along the pointer chains
We read to a uint32_t because the target process is 32 bit
*/
int memReadPtr32(HANDLE processHandle, uintptr_t address, uintptr_t *valuePtr) {
    uint32_t tempAddress = 0;
    SIZE_T bytesToRead = sizeof(tempAddress);
    SIZE_T bytesRead = 0;
    LPCVOID baseAddress = (LPCVOID)address;
    LPVOID lpBuffer = (LPVOID)&tempAddress;

    if (ReadProcessMemory(processHandle, baseAddress, lpBuffer, bytesToRead, &bytesRead) == 0) {
        DWORD error = GetLastError();
        printf("Error: memReadPtr32 failed to read pointer at address 0x%08X. Bytes read: %zu. Win32 Error: %lu\n",
               (unsigned int)address, bytesRead, error);
        return 0;
    }
    // The value is stored at tempAddress because lpBuffer holds its pointer
    *valuePtr = (uintptr_t)tempAddress;
    return 1;
}