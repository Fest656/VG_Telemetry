# AssaultCube Telemetry Tracker

A real-time hardware telemetry system that reads live game state from **AssaultCube** via the Window's API, then transmits it over serial to a **Raspberry Pi Pico** microcontroller driving physical displays.

Built as a personal project to learn  reverse engineering, Windows process internals, and embedded systems.

## How It Works

```
AssaultCube (x86)                       Host (C99 / Win API)                Raspberry Pi Pico
┌──────────────┐     ReadProcessMemory       ┌──────┐     Serial/USB     ┌──────────┐
│ Process RAM  │  ─────────────────────────► │ Host │ ─────────────────► │ Display  │
└──────────────┘  (read-only, no injection)  └──────┘   115200 baud      └──────────┘
```

1. **Reverse engineer** memory offsets with Cheat Engine
2. **Read** live values (health, armour, ammo) through static pointer chains
3. **Transmit** via ASCII serial protocol
4. **Display** on physical hardware driven by the Raspberry Pi Pico and a SSD1306



## Project Structure

```
./
├── src/
│   ├── main.c              # Entry point, main loop
│   └── memory/             # Windows process memory API
│       ├── memory.c/.h     # Attach, read, handle management
│       └── offsets.h       # Reverse-engineered addresses
├── pico/                   # Pico firmware (TBD)
├── docs/
│   ├── context.md          # RE methodology & findings
│   └── changelog.md        # Per-session development log
└── VG_Telemetry.vcxproj    # Visual Studio project
```

## Constraints

- **Read-only.** No `WriteProcessMemory`, no injection, no hooking.
- **Offline only.** Single-player mode; no network interaction.
- **Pure C99.** No C++, no third-party libraries beyond the Windows SDK.

## Status

| Phase | State |
|---|---|
| RE & Memory Mapping | ✅ Health/Armor/Ammo offset verified |
| Host Application | 🚧 Core memory reader working |
| Serial Communication | ⏳ Pending |
| Pico Firmware | ⏳ Pending |


<p align="center">
  <i>Third live telemetry test — struct populated and then formatted with the telemetry functions</i>
</p>
[https://github.com/user-attachments/assets/b9e7738f-c16e-413c-b8de-f4cf0d1f48b3](https://github.com/user-attachments/assets/8e0fed14-849b-4491-9e1c-c4d07a786033)



## License

This project is developed for academic purposes only. AssaultCube is an open-source game; no proprietary software is modified or redistributed.
