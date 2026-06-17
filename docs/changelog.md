## [13-06-2026] — Started Phase 1 (Reverse Engineering)

### Notes
- Initiated Phase 1: Reverse Engineering & Memory Mapping.
- Documented and verified the fundamental concept of static vs. dynamic memory addressing and pointer chains in `context.md`. This is crucial for understanding how to reliably read game state.
- Transitioned to Phase 2: Host Application Base. Created `offsets.h` to store the reverse-engineered addresses and started working on the memory source + header files, mainly the functions in regards to opening a handle to the game

## [14-06-2026] — Worked on Phase 2

### Notes
- Implemented Windows API wrappers for `memGetProcessId`, `memOpenProcess`, `memGetModuleBase`, and `memReadInt`.
- Resolved Visual Studio Linker errors by resolving conflicts between code editor and the IDE, along with some build configurations
- Successfully created `main.c` and executed our first live telemetry test. We traversed the pointer chain and read the live health value from AssaultCube.
- Successfully reverse-engineered the active weapon pointer logic (`0x368`), overcoming double-pointer pitfalls and inventory slot misdirection.
- Implemented real-time telemetry tracking for health, armor, active magazine ammo, and active reserve ammo.

## [14-06-2026] — Phase 3 (Hardware Integration Architecture)

### Notes
- Established the hardware architecture for the external telemetry dashboard.
- Selected Raspberry Pi Pico (RP2040) due to native USB Serial support and C SDK synergy.
- Selected SSD1306/SH1106 OLED display using the 4-pin I2C interface for simplicity.

## [17-06-2026] — Phase 4 (Serial Telemetry Implementation)

### Added
- `telemetry`: Implemented Windows API serial port configuration (`telOpenPort`, `telSetPort`) with `DCB` and `COMMTIMEOUTS` structures.
- `telemetry`: Implemented the data transmitter (`telSendState`) using `snprintf` to format the `GameState` into a positional CSV string for transmission via `WriteFile`.

### Changed
- `telemetry`: Refactored the serial protocol from a key-value pair system (`HP:100;`) to a simplified positional CSV format (`100;50;...`).
- `main`: Updated the host application main loop to populate the `GameState` struct directly and output the formatted CSV string to the console using `telStateFormat`.

### Difficulties
- `telemetry`: Encountered pointer initialization segfaults when using `LPCOMMTIMEOUTS` instead of allocating a `COMMTIMEOUTS` struct.
- `telemetry`: Order of operations bug where `SecureZeroMemory` wiped out pre-configured `DCB` settings.
- `telemetry`: Syntax errors resulting from a missing semicolon in the `telemetry.h` function declarations.

### Notes
- Decided to use standard Windows API `BOOL` over C99 `<stdbool.h>` `bool` to conform to the WINAPI.
