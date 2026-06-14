#include "memory/memory.h"
#include "memory/offsets.h"
#include <handleapi.h>
#include <stdio.h>
#include <synchapi.h>


int main() {
    // Initialize memGetProcessId
    DWORD pid;
    const char *processName = PROCESS_NAME;
    if (memGetProcessId(processName, &pid) == 0) {
        return 0;
    }
    printf( "Found PID for %s: %lu\n", processName, pid );

    // Initialize memOpenProcess
    HANDLE handle;
    if (memOpenProcess(pid, &handle) == 0) {
        return 0;
    }
    // Main is now responsible for the open handle
    printf( "Opened handle for %s: 0x%p\n", processName, (void *)handle );

    // Initialize memGetModuleBase
    uintptr_t baseAddr;
    if (memGetModuleBase(pid, processName, &baseAddr) == 0) {
        CloseHandle(handle);
        return 0;
    }
    printf( "Base address for %s: 0x%08X\n", processName, (unsigned int)baseAddr );

    int derefInt;
    if (memReadInt(handle, (baseAddr + LOCAL_PLAYER), &derefInt) == 0) {
        CloseHandle(handle);
        return 0;
    }


    int readInt;
    while (memReadInt(handle, (derefInt + OFFSET_HEALTH), &readInt) != 0) {
        printf("Health %d\n", readInt);
        Sleep(1000);
    }
    
    CloseHandle(handle);
    return 1;
}