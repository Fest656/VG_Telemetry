#include "memory/memory.h"
#include "memory/offsets.h"
#include "telemetry/telemetry.h"
#include <stdio.h>
#include <windows.h>

#define PROCESS_NAME "ac_client.exe"
#define TELEMETRY_POLL_RATE_MS 5000
#define MAX_CONSECUTIVE_FAILURES 5

// Helper function to attach to the process by name, retrieving handle and module base address.
// Returns 1 on success, 0 on failure. On success, writes through output pointer parameters.
int processAttach(const char *processName, HANDLE *handlePtr, uintptr_t *baseAddrPtr) {
    DWORD pid = 0;
    if (!memGetProcessId(processName, &pid)) {
        return 0;
    }
    // DWORD is unsigned long - lu
    printf("Found PID for %s: %lu\n", processName, pid);

    HANDLE handle = NULL;
    if (!memOpenProcess(pid, &handle)) {
        return 0;
    }
    // A HANDLE is defined as a void * so its safe to use with %p
    printf("Opened handle for %s: 0x%p\n", processName, handle);

    uintptr_t baseAddr = 0;
    if (!memGetModuleBase(pid, processName, &baseAddr)) {
        CloseHandle(handle);
        return 0;
    }
    // We need the cast because we are passing a 64 bit value to be printed as 32 bits
    printf("Base address for %s: 0x%08X\n", processName, (unsigned int)baseAddr);

    *handlePtr = handle;
    *baseAddrPtr = baseAddr;
    return 1;
}

// Helper function to find the local player address from module base address.
// Returns 1 on success, 0 on failure. On success, writes through localPlayerPtr.
// Whoever calls this function is responsible for the handle if it fails.
int getLocalPlayer(HANDLE handle, uintptr_t baseAddr, uintptr_t *localPlayerPtr) {
    uintptr_t localPlayer = 0;
    if (!memReadPtr32(handle, baseAddr + LOCAL_PLAYER, &localPlayer)) {
        return 0;
    }
    *localPlayerPtr = localPlayer;
    printf("Found local player at: 0x%08X\n", (unsigned int)localPlayer);
    return 1;
}


// Helper function to read the complete game state from the target process.
// Returns 1 on success, 0 on failure. On success, writes through statePtr.
int getGameState(HANDLE handle, uintptr_t localPlayer, GameState *statePtr) {
    uintptr_t activeWeapon = 0;
    int health = 0;
    int armor = 0;
    uintptr_t magAmmoPointer = 0;
    int magAmmo = 0;
    uintptr_t reserveAmmoPointer = 0;
    int reserveAmmo = 0;

    if (!memReadPtr32(handle, localPlayer + OFFSET_WEAPON, &activeWeapon)) {
        return 0;
    }

    if (!memReadInt(handle, localPlayer + OFFSET_HEALTH, &health)) {
        return 0;
    }

    if (!memReadInt(handle, localPlayer + OFFSET_ARMOR, &armor)) {
        return 0;
    }

    if (!memReadPtr32(handle, activeWeapon + OFFSET_MAGAMMO, &magAmmoPointer)) {
        return 0;
    }

    if (!memReadInt(handle, magAmmoPointer, &magAmmo)) {
        return 0;
    }

    if (!memReadPtr32(handle, activeWeapon + OFFSET_RESERVEAMMO, &reserveAmmoPointer)) {
        return 0;
    }

    if (!memReadInt(handle, reserveAmmoPointer, &reserveAmmo)) {
        return 0;
    }

    statePtr->health = health;
    statePtr->armor = armor;
    statePtr->magAmmo = magAmmo;
    statePtr->reserveAmmo = reserveAmmo;

    return 1;
}

int main(void) {
    // Get the handle and the base address
    HANDLE handle;
    uintptr_t baseAddr;
    if (!processAttach(PROCESS_NAME, &handle, &baseAddr)) {
        return 0;
    }

    // Get the local player
    uintptr_t localPlayer;
    if (!getLocalPlayer(handle, baseAddr, &localPlayer)) {
        CloseHandle(handle);
        return 0;
    }

    GameState state;
    int failCount = 0;
    while (1) {
        if (getGameState(handle, localPlayer, &state)) {
            failCount = 0;
            telStateFormat(&state);
        } 
        else {
            failCount++;
            printf("Warning: Failed to read complete game state this tick.\n");
        }
        if (failCount == MAX_CONSECUTIVE_FAILURES) {
            printf("Error: Failed to read game state for five ticks in a row, exiting...\n");
            break;
        }
        Sleep(TELEMETRY_POLL_RATE_MS);
    }
    
    CloseHandle(handle);
    return 1;
}

