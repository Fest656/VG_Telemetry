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

## [17-06-2026] — Phase 5 (Hardware Firmware Preparation)

### Notes
- Outlined the implementation plan for the hardware side (Raspberry Pi Pico).
- Prepared the workspace to start writing code for the Pico by installing the official VS Code SDK extension and creating a project following their guide.
- Decided on a bare-metal SSD1306 driver approach utilizing official Raspberry Pi examples to fit our use case.

## [18-06-2026] — Firmware Milestone 1 (Serial Ingestion & Parsing)

### Added
- `telemetry_display/state.h`: Created the `GameState` struct.
- `telemetry_display/serial`: Implemented `serLineRead` utilizing a non-blocking `getchar_timeout_us` polling loop to prevent the microcontroller from freezing if the serial connection drops.
- `telemetry_display/serial`: Implemented `serDataHandler` using `sscanf` to parse incoming CSV telemetry packets. Added strict validation to ensure exactly 4 integers are matched, dropping corrupted payloads.
- `telemetry_display/serial`: Implemented `serLineWrite` using standard `printf` to echo parsed data back over the virtual USB COM port for testing.

### Notes
- Decided against using `fgets` for reading serial data to avoid blocking the hardware indefinitely.
- Adapted `AGENTS.md` Rule 4 (double pointers) to use a single pointer for `serDataHandler`. This prioritizes the core philosophy of "Academic Simplicity" and readability since we are only mutating a pre-allocated struct, avoiding unnecessary pointer indirection.

## [26-06-2026] — Host Memory Refactor

### Added
- `memory.c`/`memory.h`: Added `memReadPtr` to correctly handle reading 32-bit pointers from the target process and safely casting them to `uintptr_t` for the 64-bit host environment. 

### Changed
- `main.c`: Refactored memory traversal logic to use `memReadPtr` when resolving pointer chains (e.g., `localPlayer`, `activeWeapon`), keeping `memReadInt` strictly for reading final integer values (e.g., `health`, `armor`).

## [27-06-2026] — Hardware Display Preparation
### Added
- `ssd1306.c`/`ssd1306.h`: Ported the Raspberry Pi Pico I2C ssd1306 display driver but stripped of its more complicated drawing primitives.

## [28-06-2026] — Hardware Display Implementation
### Added
- `ssd1306.c`/`ssd1306.h`: Created wrapper functions (`ssd1306_setup` and `ssd1306_render_full`) to abstract raw I2C hardware initialization and render structs, improving encapsulation.
### Changed
- `telemetry_display.c`: Integrated the SSD1306 driver into the main loop. Parses serial data, formats it into string buffers using `sprintf`, and pushes the frame to the OLED display.