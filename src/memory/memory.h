#ifndef MEMORY_H
#define MEMORY_H

#include <Windows.h>
#include <stdint.h>

/*
This file is responsible for the memory operations of the project
We will need a function to get the Process ID, one to obtain a handle to that Process ID and one that returns the base address of the process.
Then we will need the actual memory reading functions
*/

// Resolves the Process ID (PID) by executable name. 
int memGetProcessId(const char *processName, DWORD *pidPtr);

// Obtains a read-only handle to the process using its PID. 
int memOpenProcess(DWORD pid, HANDLE *handlePtr);

// Finds the base address of a module (e.g., "ac_client.exe") within the process. 
int memGetModuleBase(DWORD pid, const char *moduleName, uintptr_t *baseAddressPtr);

// Reads a 32-bit integer from the specified memory address. 
int memReadInt(HANDLE processHandle, uintptr_t address, int *valuePtr);

// Reads a 32-bit pointer address from the target process and converts it to a host-sized uintptr_t.
int memReadPtr32(HANDLE processHandle, uintptr_t address, uintptr_t *valuePtr);

#endif