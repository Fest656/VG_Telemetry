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

## [01-07-2026] — Cleanup
### Changed
- `main.c`: Code cleanup.
- `memory.c`/`telemetry.c`: Added better error handling through the usage of GetLastError for WinAPI functions.

## [08-07-2026] — Cleanup
### Changed
- `telemetry.c`: Fixed protocol field order
- `telemetry.c`/`telemetry.h`: Refactored `telOpenPort` from returning a `HANDLE` directly to the project-standard `int` status with a `HANDLE *handlePtr` output parameter.
- `telemetry.c`: Changed `dcbLen` from `DWORD` to `size_t` to match `sizeof` return type.
- `telemetry.c`: Added `snprintf` return value check for encoding errors.
- `telemetry.h`: Code cleanup by adding defined constants.
- `telemetry_display.c`: Fixed inverted return-value checks for `serLineRead` and `serDataHandler`.
- `telemetry_display.c`: Code cleanup by adding defined constants.
- `memory.c`: Fixed `memOpenProcess` writing to output parameter even on fail cases.
- `memory.c`/`memory.h`: Cleanup in their includes.
- `memory.c`: Improved error messages to include the function name and identifiers.
- `main.c`: Fixed `main()` exit codes to follow OS convention (0 = success, 1 = failure).
- `main.c`: Made the consecutive-failure error message reference `MAX_CONSECUTIVE_FAILURES`.
- `serial.c`/`serial.h`: Code cleanup by adding defined constants.
- `serial.c`: Added error messages to `serLineRead` (timeout and buffer-full paths) and `serDataHandler` (parse failure).
- `offsets.h`: Reorganized offset groupings and clarified section comments to distinguish local player offsets from active weapon offsets.
- `context.md`: Fixed stale reference `memReadPtr` → `memReadPtr32`.

## [12-07-2026] — Implemented kill/death counter

### Added
- `offsets.h`: Added `OFFSET_KILLS` (`0x1DC`) and `OFFSET_DEATHS` (`0x1E4`), both direct offsets from the LocalPlayer struct.
- `telemetry.h`/`state.h`: Added `killCount` and `deathCount` fields to the `GameState` struct (host and firmware).
- `main.c`: Added `memReadInt` calls for kills and deaths in `getGameState`. These follow the health/armor pattern (direct LocalPlayer offset, no pointer chain).

### Changed
- `telemetry.c`: Updated `telStateFormat` and `telSendState` to include kills and deaths. Serial protocol expanded from 4 fields to 6: `health;armor;magAmmo;reserveAmmo;killCount;deathCount`.
- `serial.c`: Updated `serDataHandler` to parse 6 fields and `serLineWrite` to echo all 6 values.
- `telemetry_display.c`: Redesigned OLED layout from single-column (1 value per row) to a two-column grid with fixed x,y coordinates to prevent shifting when digit counts change.

### Notes
- Kill and death values sit directly on the LocalPlayer struct (offsets `0x1DC` and `0x1E4`), so they do not need extra derenference operations like ammo values.
- The SSD1306 font (`GetFontIndex`) only supports A-Z and 0-9. Special characters (`:`, `|`, `\`) render as spaces so for now we use spacing as our formatting.

## [27-07-2026] - [29-07-2026] — Build Modes and E2E functionality

### Added
- `config.h`: Created the standard configuration file to allow users and testers to flip between build modes, refactored `main.c` and `telemetry_display.c` to use these modes.
- `main.c`: Implemented customizable COM ports through user interaction.
- `main.c`/`telemetry_display.c`: Implemented end to end functionality by allowing their interaction.

### Changed
- `main.c`: Removed the debug prints in memory read operations.
- `telemetry_display.c`: Encapsulated the frame drawing logic in a function.

[29-07-2026] - Fixes to the Echo mode

### Added
- `telemetry.c`/`telemetry.h`: Added `telReadPort` so the host app can actually read and output the data from the Pico.

### Changed
- `telemetry_display.c`: Added PICO_ECHO macro guard to the function that echoes lines back to the PC.

[02-08-2026] - Documentation changed

### Added
- `AI_usage.md`: Created in order to give an insight as to how I leveraged AI to learn the things needed for this project.

### Changed
- `context.md`: Deleted the documentation for the actual project development and kept the research done for the project.

## [07-08-2026] — Documentation Polish, MVP Audit, and First Hardware Test

### Added
- `telemetry.c`: Added explicit DCB flow control settings (`fDtrControl`, `fRtsControl`, `fOutxCtsFlow`, `fOutX`, `fInX`) to fix Error 121 (SEM_TIMEOUT).
- `telemetry_display.c`: Added `sleep_ms(2000)` after `stdio_init_all()` to allow USB CDC enumeration before entering the main loop.
- `setup.md`: Added build mode configuration step, COM port prompt mention.
- `main.c`: Added the port reading functionality to E2E mode so that the host console can receive feedback errors from the Pico.

### Changed
- `main.c`: Fixed `telSendState` return value being discarded. Rewrote the fail system to ensure every error is logged and that succesful ticks zero the fail count. Removed redundant `handleapi` import.
- `telemetry.c\telemetry.h`: Changed function signature from `telStateFormat` to `telPrintState`.
- `README.md`: Added a reference to the possibility of multithreading and caching.
- `telemetry.h`: Removed the timeout configuration so that the host app can always receive error feeback from the Pico without blocking the process.

### Tested
- First real hardware test with the Pico in `PICO_ECHO` mode. Identified and fixed USB write timeout (Error 121) caused by missing DCB flow control configuration.
