> [!NOTE]
> Everything documented here was performed completely offline.

# Phase 1: Reverse Engineering

## Research on Memory Addressing
**Finding the game's base address and offsets -> Static vs. Dynamic:**

Dynamic memory addresses mean that every time you restart the game, the address is loaded at a different location in memory. So, how do we solve this? What remains static is the **offset**—how "far" away this specific memory location is from the base address. Usually, you have a base address (the game executable itself) plus something like the local player offset. You dereference that pointer and then add the health offset to extract the health value.

The reason the base memory location changes every time the game restarts is due to a security feature called **ASLR (Address Space Layout Randomization)**. The operating system loads the game's executable (`ac_client.exe`) into a random base address in RAM. The "static" game base address we refer to is actually the relative offset from wherever the OS decided to put `ac_client.exe`.

For these efforts, we use Cheat Engine to find a dynamic memory address for the health value, and then we use the pointer scanner tool to find the static pointer chains that reliably lead to that address across game restarts. There could be multiple "paths," and we must be careful of pointer levels. We start at a minimum depth of `1` to reduce complexity.

> [!NOTE]
> Any addresses/pointers between braces [] mean they are supposed to be dereferenced

### The Process of Searching for the DMA (Dynamic Memory Address) for Health:
1. Attach Cheat Engine to the game process.
2. Since health is most likely an integer, configure the search for a 4-byte value.
3. Start with an exact value scan of `100` (default health). Narrow this down by taking damage in-game and using the "Next Scan" function with the new health value.
4. This second scan yielded the address `0x007EB77C`. This address is dynamic (changes upon game restart), but we verified its correctness by freezing the value in Cheat Engine and confirming we took no damage in-game.
5. We then used Cheat Engine's Pointer Scan function on this address to find the base address and offset chain, setting the maximum pointer depth to 1 to begin with.
6. This yielded 4 results. We took damage in-game to verify they updated correctly, and then restarted the game entirely while keeping the address list to verify that the pointers still reliably resolved to the new health address.
7. From these 4 stable pointers, we selected the following path: 
   `[ac_client.exe + 0x17E0A8] + 0xEC -> Health Address`.

> [!TIP]
> We can repeat this procedure to find other offsets, such as primary ammo, magazine ammo, armor, and others. Now that the local_player address is known it is a lot easier.

> [!WARNING]
> Cheat Engine displays memory addresses in Hexadecimal, but displays pointer *values* in Decimal by default. When working between tools like `Dissect data/structure` so we need to be careful when working with static/dynamic pointers/addresses to access the information correctly.

### The Process of Searching for the DMA (Dynamic Memory Address) for active ammo:
Searching for the active weapon's ammo is more complex because the game engine does not store the ammo directly in the weapon. Instead, we use the debugger to trace the assembly instructions:

1. Since the active magazine ammo is an integer, configure the search for a 4-byte value.
2. Start with an exact value scan of the current magazine ammo (e.g., `20`). Narrow this down by firing the weapon in-game and using the "Next Scan" function.
3. This yielded the dynamic address for the active magazine ammo. To understand how the game calculates it, we right-clicked the address and selected **"Find out what accesses this address"**.
4. Upon shooting the gun in-game, the debugger caught the following instructions:
   ```assembly
   mov eax, [esi+14]
   dec [eax]
   ```
   This reveals that the game retrieves the ammo pointer from an offset of `0x14` inside the Weapon Object (which was temporarily stored in the `ESI` register).
5. Checking the instruction's register info revealed `ESI` was `0x00714DA8`. We then performed a 4-byte **Hex Scan** for this exact value to find the static offset leading to the Weapon Object.
6. This scan yielded an address perfectly nested inside the **LocalPlayer** object at offset `0x344` (which we later found out was the Carbine).
7. To find the **Reserve Ammo**, we repeated this entire procedure: scanning for the reserve ammo value, attaching the debugger, and reloading the gun.
8. The debugger caught the following instructions:
   ```assembly
   mov eax, [edi+10]
   sub [eax], esi
   ```
   This proved that the reserve ammo pointer sits directly next to the magazine pointer at offset `0x10` inside the Weapon Object.
9. After a restart, we discovered that we had only figured out the address to the Carbine slot, which meant this did not work across all weapons. `0x344` was how you access the carbine, but now we knew that the offset `0x344` was pointing to a specific weapon.
10. Knowing this, we dissected the LocalPlayer structure in Cheat Engine's Dissect Data tool to see the memory address that `0x344` pointed to. We then searched nearby offsets to find what else pointed to that exact same address.
11. We then had a couple of consecutive suspects, and it was a matter of switching the active weapon in-game to see which ones dynamically updated. That's when we got `0x368`.
12. So the final paths we were left with were:
   `[[[ac_client.exe + 0x17E0A8] + 0x368] + 0x14] -> Ammo in magazine`
   `[[[ac_client.exe + 0x17E0A8] + 0x368] + 0x10] -> Ammo in reserve`
---

# Phase 2: Host Application Base

We created a C program that reads the memory of the game and extracts the data we need into the console. For this project, we used **Windows API functions** to achieve this.

We decided to use `#define` macros to handle the storage of our addresses since these are known at compile time.

## Data Types in Windows API Memory Operations
To interact safely with the Windows API and process memory, we use specific data types instead of standard C types (like `int` or `long`):

* **`DWORD` (Double Word):** A 32-bit unsigned integer provided by `<Windows.h>`. It is the standard type the Windows API uses to represent system identifiers like Process IDs (PIDs) and sizes.
* **`HANDLE`:** An opaque pointer used by Windows to represent an open instance of a system resource (in our case, the `ac_client.exe` process). We obtain this via `OpenProcess` and must use it to authorize any subsequent `ReadProcessMemory` operations.
* **`uintptr_t`:** An unsigned integer type from `<stdint.h>` that is guaranteed to be the exact same size as a pointer on the target architecture (32 bits for an x86 process, 64 bits for an x64 process). Because we are doing pointer arithmetic (adding offsets to base addresses), doing math directly on raw pointers can be dangerous or violate C standards. Storing addresses as `uintptr_t` allows us to perform safe hexadecimal arithmetic.
* **`LPCVOID` (Long Pointer to Constant Void):** A pointer to a read-only memory location (`const void *`). `ReadProcessMemory` uses this type for the source address parameter — the address inside the target process we want to read from. We cast our `uintptr_t` address to `LPCVOID` before passing it.
* **`LPVOID` (Long Pointer to Void):** A generic pointer to any memory location (`void *`). `ReadProcessMemory` uses this type for the destination buffer parameter — the local variable where the read data will be written into. We cast our output pointer to `LPVOID` before passing it.

## The Memory Source & Header Files
We implemented a custom memory API wrapper using `CreateToolhelp32Snapshot`, `OpenProcess`, and `ReadProcessMemory`. A key design decision was **handle ownership**: functions that create temporary handles (like snapshot handles) are responsible for closing them before returning, while `memOpenProcess` transfers ownership of the process handle to its caller, which must call `CloseHandle` when done.

The four functions and their responsibilities:
* **`memGetProcessId`** — Takes a snapshot of all running processes, iterates through them to find a match by name, and writes the PID to an output pointer. The snapshot handle is created and closed entirely within this function.
* **`memOpenProcess`** — Calls `OpenProcess` with `PROCESS_VM_READ` (read-only access) and writes the resulting handle to an output pointer. This function is **not** responsible for the handle — the caller owns it.
* **`memGetModuleBase`** — Takes a module snapshot of the target process (using `TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32` to handle our 64-bit host reading a 32-bit process), iterates to find the module by name, and writes its base address as a `uintptr_t`. The snapshot handle is created and closed within this function.
* **`memReadInt`** — Wraps `ReadProcessMemory` to read a single `int` from a given address. Casts the `uintptr_t` address to `LPCVOID` and the output pointer to `LPVOID` to satisfy the Windows API signature.

Our testing in `main.c` proved that we can successfully traverse the pointer chain (`[ac_client.exe + 0x17E0A8] + 0xEC`) by performing two sequential memory reads:

Now we were onto more difficult challenges such as the ammo counters. As described in the reverse engineering section, we had some troubles that were resolved through some more intelligent use of the tools at our disposal. The host application also suffered some issues: we fell into some pointer pitfalls when reading ammo values because we forgot to dereference the final pointer to access the real value.

> [!WARNING]
> VG_Telemetry must be initiated and tested in the **base lobby** for the game, otherwise health always reads zero. This is a quirk of how AssaultCube initializes the local player object in memory.

---

# Phase 3: Hardware Integration Architecture

To achieve the hardware/embedded programming part of this project we decided to integrate an external microcontroller and an OLED screen.

**Hardware Stack:**
*   **Microcontroller:** Raspberry Pi Pico (RP2040)
*   **Display:** SSD1306 or SH1106 OLED (4-Pin I2C version)

**Architectural Flow:**
1.  **Host PC (`VG_Telemetry.exe`):** The Windows C program continues reading live telemetry (health, armor, active ammo) from the game memory. It then opens a standard Serial Port (COM) connection to the Pico via USB and sends the telemetry data as formatted packets.
2.  **Raspberry Pi Pico:** Running a C/C++ firmware program, the Pico acts as a native USB Serial device. It runs an infinite loop waiting to receive and parse the incoming serial packets from the host PC.
3.  **OLED Display (I2C):** The Pico utilizes its hardware I2C controllers (using only 4 wires: VCC, GND, SDA, and SCL) to send drawing commands to the OLED screen. This allows us to display live health bars and ammo counters externally, completely decoupled from the game's rendering engine.

The decision to use the Raspberry Pi Pico over a WiFi-enabled MCU like the ESP32 was driven by its robust native USB support for Serial communication and seamless integration with C/C++ SDKs, keeping the entire project unified under the C programming standard.

---

# Phase 4: Serial Telemetry Implementation

With the memory extraction working, we needed a way to transmit the `GameState` out of the host PC. We built a custom serial transmission module (`telemetry.c` / `telemetry.h`) utilizing the Windows API.

## Windows API Serial Communication
Just like reading memory, manipulating hardware ports in Windows requires requesting handles and configuring low-level structs. We implemented three core functions:

1. **`telOpenPort`:** We use `CreateFile` (with `GENERIC_READ | GENERIC_WRITE`) to request an I/O handle to the target COM port. If successful, Windows grants us exclusive access to the USB serial connection.
> [!IMPORTANT]
> **Handle Ownership Rule:** Just like our memory API (`memOpenProcess`), any function in this project that successfully opens a Windows `HANDLE` and returns it transfers ownership to the caller. The caller (typically `main.c`) is always responsible for eventually calling `CloseHandle()`.
2. **`telSetPort`:** Serial ports require strict synchronization between sender and receiver. We use the `GetCommState` and `SetCommState` functions to inject our configuration into the port's Device Control Block (`DCB`), explicitly setting the baud rate to `115200`, 8 data bits, no parity, and 1 stop bit. Crucially, we also initialize a `COMMTIMEOUTS` struct and apply it via `SetCommTimeouts` to ensure `WriteFile` operations do not block the thread indefinitely if the Pico is disconnected.
3. **`telSendState`:** To minimize dynamic memory allocation and the potential issues that come with it, we format the outgoing data using `snprintf` into an array (`char buffer[64]`). We then use `WriteFile` to push the buffer out through the COM handle.

## Host Application Integration
As a bridging step before connecting the physical microcontroller, we updated the host application's main loop in `main.c` to populate the `GameState` struct directly. We bypassed the serial transmission temporarily and used `telStateFormat` to output the formatted CSV string directly to the console. This allowed us to visually verify that the memory extraction and protocol formatting logic were perfectly synchronized in real-time.
