#include "memory/memory.h"
#include "memory/offsets.h"
#include "telemetry/telemetry.h"
#include <stdio.h>
#include <windows.h>

#define PROCESS_NAME "ac_client.exe"

int main() {
    // Initialize memGetProcessId
    DWORD pid;
    const char *processName = PROCESS_NAME;
    if (memGetProcessId(processName, &pid) == 0) {
        return 0;
    }
    printf("Found PID for %s: %lu\n", processName, pid);

    // Initialize memOpenProcess
    HANDLE handle;
    if (memOpenProcess(pid, &handle) == 0) {
        return 0;
    }
    // Main is now responsible for the open handle
    printf("Opened handle for %s: 0x%p\n", processName, (void *)handle);

    // Initialize memGetModuleBase
    uintptr_t baseAddr;
    if (memGetModuleBase(pid, processName, &baseAddr) == 0) {
        CloseHandle(handle);
        return 0;
    }
    printf("Base address for %s: 0x%08X\n", processName, (unsigned int)baseAddr);

    uintptr_t localPlayer;
    if (memReadPtr(handle, baseAddr + LOCAL_PLAYER, &localPlayer) == 0) {
        CloseHandle(handle);
        return 0;
    }
    printf("Found local player at: 0x%08X\n", (unsigned int)localPlayer);

    GameState state;
    uintptr_t activeWeapon;
    uintptr_t magAmmoPointer;
    uintptr_t reserveAmmoPointer;

    while (1) {
        // Read active weapon pointer
        if (memReadPtr(handle, localPlayer + OFFSET_WEAPON, &activeWeapon) == 0) {
            printf("Failed to read the active weapon!\n");
            CloseHandle(handle);
            return 0;
        }

        // Read health
        memReadInt(handle, localPlayer + OFFSET_HEALTH, &state.health);

        // Read armor
        memReadInt(handle, localPlayer + OFFSET_ARMOR, &state.armor);

        // Read mag ammo (requires another dereference)
        if (memReadPtr(handle, activeWeapon + OFFSET_MAGAMMO, &magAmmoPointer) != 0) {
            memReadInt(handle, magAmmoPointer, &state.magAmmo);
        }

        // Read reserve ammo (requires another dereference)
        if (memReadPtr(handle, activeWeapon + OFFSET_RESERVEAMMO, &reserveAmmoPointer) != 0) {
            memReadInt(handle, reserveAmmoPointer, &state.reserveAmmo);
        }

        telStateFormat(&state);
        Sleep(5000);
    }
    
    CloseHandle(handle);
    return 1;
}
