# AssaultCube Telemetry Tracker

A real-time hardware telemetry system that reads live game state from **AssaultCube** via the Window's API, then transmits it over serial to an **ESP32** microcontroller driving physical displays.

Built as a personal project to learn  reverse engineering, Windows process internals, and embedded systems.

## How It Works

```
AssaultCube (x86)                       Host (C99 / Win API)                ESP32
┌──────────────┐     ReadProcessMemory       ┌──────┐     Serial/USB     ┌──────────┐
│ Process RAM  │  ─────────────────────────► │ Host │ ─────────────────► │ Display  │
└──────────────┘  (read-only, no injection)  └──────┘   115200 baud      └──────────┘
```

1. **Reverse engineer** memory offsets with Cheat Engine
2. **Read** live values (health, armour, ammo) through static pointer chains
3. **Transmit** via ASCII serial protocol
4. **Display** on physical hardware driven by the ESP32



## Project Structure

```
./
├── src/
│   ├── main.c              # Entry point, main loop
│   └── memory/             # Windows process memory API
│       ├── memory.c/.h     # Attach, read, handle management
│       └── offsets.h       # Reverse-engineered addresses
├── esp32/                  # ESP32 firmware (TBD)
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
| ESP32 Firmware | ⏳ Pending |


<p align="center">
  <i>First live telemetry test — health value read in real time</i><br><br>
  <img src="docs/Telemetry_testn1.gif" width="700"/>
</p>

<p align="center">
  <i>Second live telemetry test — ammo values read in real time</i><br><br>
  <a href="https://github.com/user-attachments/assets/c678a5b5-b397-49c4-a5ff-78664d6f25ea">
    <img src="https://github.com/user-attachments/assets/c678a5b5-b397-49c4-a5ff-78664d6f25ea" width="700"/>
  </a>
</p>

## License

This project is developed for academic purposes only. AssaultCube is an open-source game; no proprietary software is modified or redistributed.