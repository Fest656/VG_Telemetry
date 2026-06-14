#include "memory.h"
#include <tlhelp32.h> 
#include <stdio.h>    
#include <string.h>   
#include <stdbool.h>  


/*
https://learn.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-createtoolhelp32snapshot
https://learn.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-process32first
https://learn.microsoft.com/en-us/windows/win32/api/tlhelp32/ns-tlhelp32-processentry32
*/
int memGetProcessId( const char *processName, DWORD *pidPtr ) {
    HANDLE handleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 
    if (handleSnapshot == INVALID_HANDLE_VALUE) {
        printf("Error: Could not create system process snapshot.\n");
        return 0;
    }
    // At this point we assume we have a handleSnapshot for which we are responsible
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    bool hasNext = Process32First(handleSnapshot, &entry);
    if(hasNext != TRUE) {
        printf("Error: Could not get process first entry.\n");
        CloseHandle(handleSnapshot);
        return 0;
    }

    while (hasNext == TRUE) {
        if ((strcmp(entry.szExeFile, processName)) == 0 )  {
            *pidPtr = entry.th32ProcessID;
            CloseHandle(handleSnapshot);
            return 1; // Found the process, assign to argument pidPtr
        }
        hasNext = Process32Next(handleSnapshot, &entry);
    }

    // Process was not found
    printf("Error: Process was not found.\n");
    CloseHandle(handleSnapshot);
    return 0;
}   

/*
https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-openprocess
https://learn.microsoft.com/en-us/windows/win32/procthread/process-security-and-access-rights
https://learn.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-closehandle
*/
int memOpenProcess( DWORD pid, HANDLE *handlePtr ) {
    *handlePtr = OpenProcess(PROCESS_VM_READ, FALSE, pid);
    if (*handlePtr == NULL) {
        printf("Error: Could not open process.\n");
        return 0;
    }
    // This function is not responsible for the handle
    return 1;
}

/*
Very similar to getting the PID, the difference being the keywords used in CreateToolhelp32Snapshot
We use TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32 because this process is 64 bit and assault cube is a 32 bit application
*/
int memGetModuleBase( DWORD pid, const char *moduleName, uintptr_t *baseAddressPtr ){
    HANDLE handleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid); 
    if (handleSnapshot == INVALID_HANDLE_VALUE) {
        printf("Error: Could not create system process snapshot.\n");
        return 0;
    }
    // At this point we assume we have a handleSnapshot for which we are responsible

    MODULEENTRY32 entry;
    entry.dwSize = sizeof(MODULEENTRY32);
    bool hasNext = Module32First(handleSnapshot, &entry);
    if(hasNext != TRUE) {
        printf("Error: Could not get module first entry.\n");
        CloseHandle(handleSnapshot);
        return 0;
    }

    while (hasNext == TRUE) {
        if ((strcmp(entry.szModule, moduleName)) == 0 )  {
            *baseAddressPtr = (uintptr_t)entry.modBaseAddr; // Need to cast
            CloseHandle(handleSnapshot);
            return 1; // Found the module, assign to argument baseAddressPtr
        }
        hasNext = Module32Next(handleSnapshot, &entry);
    }

    // Module was not found
    printf("Error: Module was not found.\n");
    CloseHandle(handleSnapshot);
    return 0;
}

/*
https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-readprocessmemory
*/
int memReadInt( HANDLE processHandle, uintptr_t address, int *valuePtr ) {
    // The function expects an LPCVOID address, so we need to cast
    SIZE_T bytesToRead = sizeof(int);
    SIZE_T bytesRead = 0;
    LPCVOID baseAddress = (LPCVOID)address;
    LPVOID lpBuffer = (LPVOID)valuePtr;

    if (ReadProcessMemory(processHandle, baseAddress, lpBuffer, bytesToRead, &bytesRead) == 0) {
        printf("Error: Could not read memory\n.");
        return 0;
    }
    return 1;
}