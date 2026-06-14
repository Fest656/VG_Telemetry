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
