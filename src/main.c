#include "memory/memory.h"
#include "memory/offsets.h"
#include "telemetry/telemetry.h"
#include <handleapi.h>
#include <stdio.h>
#include <windows.h>
#include "../config/config.h"

#define PROCESS_NAME "ac_client.exe"
#define TELEMETRY_POLL_RATE_MS   250
#define MAX_CONSECUTIVE_FAILURES 5

// Helper function to attach to the process by name, retrieving handle and module base address.
// Returns 1 on success, 0 on failure. On success, writes through output pointer parameters.
// Caller is responsible for the handle
static int processAttach(const char *processName, HANDLE *handlePtr, uintptr_t *baseAddrPtr) {
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
    printf("Opened handle for %s: %p\n", processName, handle);

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
static int getLocalPlayer(HANDLE handle, uintptr_t baseAddr, uintptr_t *localPlayerPtr) {
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
static int getGameState(HANDLE handle, uintptr_t localPlayer, GameState *statePtr) {
    uintptr_t activeWeapon = 0;
    int health = 0;
    int armor = 0;
    int kills = 0;
    int deaths = 0;
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

    if (!memReadInt(handle, localPlayer + OFFSET_KILLS, &kills)) {
        return 0;
    }

    if (!memReadInt(handle, localPlayer + OFFSET_DEATHS, &deaths)) {
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
    statePtr->killCount = kills;
    statePtr->deathCount = deaths;

    return 1;
}

// 0 for success && 1 for failure
int main(void) {
    // Get the handle and the base address
    HANDLE processHandle;
    uintptr_t baseAddr;
    if (!processAttach(PROCESS_NAME, &processHandle, &baseAddr)) {
        return 1;
    }
    // Main is now responsible for the process handle

    uintptr_t localPlayer;
    if (!getLocalPlayer(processHandle, baseAddr, &localPlayer)) {
        CloseHandle(processHandle);
        return 1;
    }

    // COM port initialization, only for E2E or Pico Echo
    #if HAS_COM_PORT
    HANDLE comHandle;
    char port[5];
    unsigned int portBuffer = (unsigned int)sizeof(port);
    printf("Enter COM port (works for COM1 - COM9):\n");
    if (scanf_s("%s", port, portBuffer) != 1) {
        printf("Error: Failed to read COM port\n");
        CloseHandle(processHandle);
        return 1;
    }
    if (!telOpenPort(port, &comHandle)) {
        CloseHandle(processHandle);
        return 1;
    }
    // Main is now responsible to the COM port handle
    if (!telSetPort(comHandle)) {
        CloseHandle(processHandle);
        CloseHandle(comHandle);
        return 1;
    }
    #endif

    GameState state;
    int failCount = 0;
    while (1) {
        if (getGameState(processHandle, localPlayer, &state)) {
            failCount = 0;
            #if BUILD_MODE == CONSOLE
            telStateFormat(&state);
            #else
            telSendState(&state, comHandle);
            #if BUILD_MODE == PICO_ECHO
            telReadPort(comHandle);
            #endif
            #endif
        }
        else {
            failCount++;
            printf("Warning: Failed to read complete game state this tick.\n");
        }

        if (failCount == MAX_CONSECUTIVE_FAILURES) {
            printf("Error: Failed to read game state for %d ticks in a row, exiting...\n",
                   MAX_CONSECUTIVE_FAILURES);
            break;
        }
        
        Sleep(TELEMETRY_POLL_RATE_MS);
    }
    #if HAS_COM_PORT
    CloseHandle(comHandle);
    #endif
    CloseHandle(processHandle);
    return 0;
}
