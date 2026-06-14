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

    int localPlayer;
    if (memReadInt(handle, (baseAddr + LOCAL_PLAYER), &localPlayer) == 0) {
        CloseHandle(handle);
        return 0;
    }
    printf("Found local player at: %d\n", localPlayer);



    int activeWeapon;
    //int playerHealth;
    //int playerArmor;
    int magAmmoPointer;
    int magazineAmmo;
    int reserveAmmoPointer;
    int reserveAmmo;
    while(1) {
        
        // Read active weapon
        if (memReadInt(handle, (localPlayer + OFFSET_WEAPON), &activeWeapon) == 0) {
            printf("Failed to read the active weapon!\n");
            CloseHandle(handle);
            return 0;
        }
        /*
        // Read health
        if (memReadInt(handle, (localPlayer + OFFSET_HEALTH), &playerHealth) != 0) {
            printf("Health %d\n", playerHealth);
        }
        // Read armor
        if (memReadInt(handle, (localPlayer + OFFSET_ARMOR), &playerArmor) != 0) {
            printf("Armor %d\n", playerArmor);
        }
        */
        // Read mag ammo
        if (memReadInt(handle, (activeWeapon + OFFSET_MAGAMMO), &magAmmoPointer) != 0) {
            // Dereference
            if (memReadInt(handle, magAmmoPointer, &magazineAmmo) != 0) {
                printf("Ammo in mag: %d\n", magazineAmmo);
            }
        }

        if (memReadInt(handle, (activeWeapon + OFFSET_RESERVEAMMO), &reserveAmmoPointer) != 0) {
            // Dereference
            if (memReadInt(handle, reserveAmmoPointer, &reserveAmmo) != 0) {
                printf("Ammo in reserve: %d\n", reserveAmmo);
            }
        }

        Sleep(5000);
    }
    
    CloseHandle(handle);
    return 1;
}