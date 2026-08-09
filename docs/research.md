> [!NOTE]
> Everything documented here was performed completely offline and on assault cube version `1.3.0.2`.

# Reverse Engineering

## Research on Memory Addressing

**Finding the game's base address and offsets -> Static vs. Dynamic:**

Dynamic memory addresses mean that every time the game restarts, the address is loaded at a different location in memory. What remains static is the **offset** — how "far" away this specific memory location is from the base address. Usually, there is a base address (the game executable itself) plus something like the LocalPlayer struct offset. Dereferencing that pointer and adding the health offset extracts the health value.

The reason the base memory location changes every time the game restarts is due to a security feature called **ASLR (Address Space Layout Randomization)**. The operating system loads the game's executable (`ac_client.exe`) into a random base address in RAM. The "static" game base address is actually the relative offset from wherever the OS decided to put `ac_client.exe`.

To find stable pointer chains, Cheat Engine was used to locate a dynamic memory address for the health value. The pointer scanner tool then identified the static chains that reliably lead to that address even across game restarts.

> [!NOTE]
> Any addresses/pointers between braces [] mean they are supposed to be dereferenced.

## The Process of Searching for the DMA (Dynamic Memory Address) for Health:
1. Attach Cheat Engine to the game process.
2. Configure the search for a 4-byte value, as health is typically represented as an integer.
3. Start with an exact value scan of `100` (default health). Narrow this down by taking damage in-game and using the "Next Scan" function with the new health value.
4. This method yields a dynamic address (e.g., `0x007EB77C`). Correctness is verified by freezing the value in Cheat Engine and confirming immunity to damage in-game.
5. Execute Cheat Engine's Pointer Scan on this address to find the base address and offset chain, initially setting the maximum pointer depth to 1.
6. This yields potential stable chains. Taking damage in-game verifies they update correctly. Restarting the game entirely verifies the pointers still reliably resolve to the new health address.
7. From these stable pointers, the following path was selected: 
   `[ac_client.exe + 0x17E0A8] + 0xEC -> Health Address`.

> [!TIP]
> Once the LocalPlayer struct address is found, this procedure can be repeated to find other offsets.


## The Process of Searching for the DMA (Dynamic Memory Address) for active ammo:
Searching for the active weapon's ammo is more complex because the game engine stores the ammo indirectly. The debugger was used to trace the assembly instructions:

1. Execute the same steps as the health search above.
2. This yields the dynamic address for the active magazine ammo. To understand how the game calculates it, right-click the address and select "Find out what accesses this address".
3. Upon shooting the gun in-game, the debugger catches the following instructions:
   ```assembly
   mov eax, [esi+14]
   dec [eax]
   ```
   This reveals that the game retrieves the ammo pointer from an offset of `0x14` inside the Weapon Object (temporarily stored in the `ESI` register).
4. Checking the instruction's register info reveals `ESI` was `0x00714DA8`. A 4-byte Hex Scan for this exact value finds the static offset leading to the Weapon Object.
5. This scan yields an address nested inside the LocalPlayer object at offset `0x344` (specific to the Carbine).
6. To find the **Reserve Ammo**, this procedure is repeated: scanning for the reserve ammo value, attaching the debugger, and reloading the gun.
7. The debugger catches the following instructions:
   ```assembly
   mov eax, [edi+10]
   sub [eax], esi
   ```
   This proves that the reserve ammo pointer sits directly next to the magazine pointer at offset `0x10` inside the Weapon Object.
8. After a restart, it was discovered that `0x344` only resolved to the Carbine slot, meaning it did not work across all weapons. The offset `0x344` pointed to a specific weapon, not the actively held weapon.
9. Dissecting the LocalPlayer structure in Cheat Engine's Dissect Data tool revealed the memory address that `0x344` pointed to. Searching nearby offsets identified what else pointed to that exact same address.
10. Switching the active weapon in-game revealed which consecutive offsets dynamically updated, identifying `0x368` as the active weapon pointer.
11. The final resolved paths are:
   `[[[ac_client.exe + 0x17E0A8] + 0x368] + 0x14] -> Ammo in magazine`
   `[[[ac_client.exe + 0x17E0A8] + 0x368] + 0x10] -> Ammo in reserve`
   
> [!WARNING]
> Cheat Engine displays memory addresses in Hexadecimal, but displays pointer *values* in Decimal by default. When working between tools like `Dissect data/structure`, care must be taken to access the information correctly.

---

# Serial Communication

## Research on Serial Port Communication

**How the PC running the host app talks to the MCU:**
Serial communication is one of the simplest methods for exchanging data between devices.
At its core, it transmits data sequentially on the wire asynchronously. Given that specification, certain communication configurations must be agreed upon before the communication port is established, such as:

* **Baud rate:** The baud rate is set to `115200`, matching the Pico SDK default for USB serial.
* **Data bits:** Data bits are set to 8, the standard size for a byte.
* **Parity:** `NOPARITY` is used. Given the short USB connection and low data throughput, hardware error detection is unnecessary.
* **Stop bits:** Set to `ONESTOPBIT` to signal the end of each transmitted character.
  > [!NOTE]
   > These are set by the **Device Control Block (DCB)**, which is a struct that holds the serial configuration. They are applied with `SetCommState`.

## Serial Port Communication on Windows

On Windows machines, serial communication ports can be accessed in usermode as **COM Ports**, accessible as resources named `COMX` with X being the number of the COM port.
A COM port is mapped to a physical interface. The Win32 API treats them like files, meaning they can be opened with `CreateFile`, read with `ReadFile`, and written to with `WriteFile`. The handle must still be closed with `CloseHandle`.
   > [!TIP]
   > Windows has a naming quirk with COM ports. Ports COM1 through COM9 can be opened by name directly, but ports numbered COM10 and above require the device path prefix: `\\.\COM10`. 

## How the MCU becomes a COM Port

In this project, the RP2040 implements **USB CDC (Communications Device Class)** in firmware, emulating a serial device entirely. From the host PC's perspective, plugging in the Pico creates a new COM port automatically.
This means the Pico's "serial port" is actually a USB endpoint pretending to be one. The practical consequence is that the baud rate setting on the host side is irrelevant for actual transmission speed, as USB CDC ignores it and transmits at USB speeds. It is still configured to `115200` because the Windows API requires a valid DCB configuration.

> [!NOTE]
> When the Pico SDK is configured with `pico_enable_stdio_usb(project 1)`, the standard C `printf` and `getchar` functions are redirected through this virtual USB serial interface.

# Project Compilation and Build Modes

## Build Mode Options

* **`E2E` (End-to-End):** Standard production pipeline. The host application reads game memory, formats data into CSV packets, and transmits them over COM port to the Pico. The Pico parses the data and renders it to the SSD1306 display.
* **`PICO_ECHO`:** Serial loopback testing mode. The host application transmits telemetry over COM port to the Pico, and the Pico echoes parsed values back to the host via USB serial (`serLineWrite`) for protocol verification.
* **`CONSOLE`:** Host-only debugging mode. Telemetry is read from memory and formatted directly to the PC console (`telStateFormat`). Serial COM port initialization is disabled (`HAS_COM_PORT` evaluates to `0`), allowing host testing without hardware attached.

## Implementation

A central `#define BUILD_MODE` macro is defined in `config/config.h` to drive conditional compilation (`#if HAS_COM_PORT` and `#if BUILD_MODE == ...`). This allows standalone testing across all three layers, with the tradeoff of requiring recompilation when switching modes.