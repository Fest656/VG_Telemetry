#ifndef MEMORY_H
#define MEMORY_H

#include <windows.h>
#include <stdint.h>

/*
Process memory reading API.
All functions return 1 on success, 0 on failure.
Output parameters are only written on the success path.
*/

// Resolves the Process ID (PID) by executable name. Writes through pidPtr on success.
int memGetProcessId(const char *processName, DWORD *pidPtr);

// Obtains a read-only handle to the process identified by pid.
// On success, writes through handlePtr. Caller is responsible for calling CloseHandle on the returned handle.
int memOpenProcess(DWORD pid, HANDLE *handlePtr);

// Finds the base address of a module (e.g., "ac_client.exe") within the target process.
// Writes through baseAddressPtr on success.
int memGetModuleBase(DWORD pid, const char *moduleName, uintptr_t *baseAddressPtr);

// Reads a 32-bit integer from the target process at the specified address. Writes through valuePtr on success.
int memReadInt(HANDLE processHandle, uintptr_t address, int *valuePtr);

// Reads a 32-bit pointer from the target process and widens it to host-sized uintptr_t.
// Designed for traversing pointer chains in a 32-bit target from a 64-bit host.
int memReadPtr32(HANDLE processHandle, uintptr_t address, uintptr_t *valuePtr);

#endif