> [!NOTE]
> Everything documented here was performed completely offline.

# Phase 1: Reverse Engineering

## Research on Memory Addressing
**Finding the game's base address and offsets -> Static vs. Dynamic:**

Dynamic memory addresses mean that every time you restart the game, the address is loaded at a different location in memory. So, how do we solve this? What remains static is the **offset**—how "far" away this specific memory location is from the base address. Usually, you have a base address (the game executable itself) plus something like the local player offset. You dereference that pointer and then add the health offset to extract the health value.

The reason the base memory location changes every time the game restarts is due to a security feature called **ASLR (Address Space Layout Randomization)**. The operating system loads the game's executable (`ac_client.exe`) into a random base address in RAM. The "static" game base address we refer to is actually the relative offset from wherever the OS decided to put `ac_client.exe`.

For these efforts, we use Cheat Engine to find a dynamic memory address for the health value, and then we use the pointer scanner tool to find the static pointer chains that reliably lead to that address across game restarts. There could be multiple "paths," and we must be careful of pointer levels. We start at a minimum depth of `1` to reduce complexity.

### The Process of Searching for the DMA (Dynamic Memory Address) for Health:
1. Attach Cheat Engine to the game process.
2. Since health is most likely an integer, configure the search for a 4-byte value.
3. Start with an exact value scan of `100` (default health). Narrow this down by taking damage in-game and using the "Next Scan" function with the new health value.
4. This second scan yielded the address `0x007EB77C`. This address is dynamic (changes upon game restart), but we verified its correctness by freezing the value in Cheat Engine and confirming we took no damage in-game.
5. We then used Cheat Engine's Pointer Scan function on this address to find the static base address and offset chain, setting the maximum pointer depth to 1 to begin with.
6. This yielded 4 results. We took damage in-game to verify they updated correctly, and then restarted the game entirely while keeping the address list to verify that the pointers still reliably resolved to the new health address.
7. From these 4 stable pointers, we selected the following path: 
   `[ac_client.exe + 0x17E0A8] + 0xEC -> Health Address`.

> [!TIP]
> We can repeat this procedure to find other offsets, such as primary ammo, magazine ammo, armor, and others.

---

# Phase 2: Host Application Base

We created a C program that reads the memory of the game and extracts the data we need into the console. For this project, we used **Windows API functions** to achieve this.

We decided to use `#define` macros to handle the storage of our addresses since these are known at compile time.

## Data Types in Windows API Memory Operations
To interact safely with the Windows API and process memory, we use specific data types instead of standard C types (like `int` or `long`):

* **`DWORD` (Double Word):** A 32-bit unsigned integer provided by `<Windows.h>`. It is the standard type the Windows API uses to represent system identifiers like Process IDs (PIDs) and sizes.
* **`HANDLE`:** An opaque pointer used by Windows to represent an open instance of a system resource (in our case, the `ac_client.exe` process). We obtain this via `OpenProcess` and must use it to authorize any subsequent `ReadProcessMemory` operations.
* **`uintptr_t`:** An unsigned integer type from `<stdint.h>` that is guaranteed to be the exact same size as a pointer on the target architecture (32 bits for an x86 process, 64 bits for an x64 process). Because we are doing pointer arithmetic (adding offsets to base addresses), doing math directly on raw pointers can be dangerous or violate C standards. Storing addresses as `uintptr_t` allows us to perform safe hexadecimal arithmetic.

## The Memory Source & Header Files
We successfully implemented a custom memory API wrapper using `CreateToolhelp32Snapshot`, `OpenProcess`, and `ReadProcessMemory`. Our testing in `main.c` proved that we can successfully traverse the pointer chain (`[ac_client.exe + 0x17E0A8] + 0xEC`) by performing two sequential memory reads:

1. Reading the base pointer to get the dynamic player object address.
2. Adding `OFFSET_HEALTH` to the player object address and reading the final integer.

> [!WARNING]
> VG_Telemetry must be initiated and tested in the **base lobby** for the game, otherwise health always reads zero. This is a quirk of how AssaultCube initializes the local player object in memory.