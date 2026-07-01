# AssaultCube Telemetry Tracker

A real-time hardware telemetry system that reads live game state from **AssaultCube** via the Window's API, then transmits it over serial to a **Raspberry Pi Pico** microcontroller driving physical displays.

Built as a personal project to learn  reverse engineering, Windows process internals, and embedded systems.

## How It Works

```
AssaultCube (x86)                       Host (C / Win API)            Raspberry Pi Pico
┌──────────────┐     ReadProcessMemory       ┌──────┐     Serial/USB     ┌──────────┐
│ Process RAM  │  ─────────────────────────► │ Host │ ─────────────────► │ Display  │
└──────────────┘       (read-only)           └──────┘    115200 baud     └──────────┘
```

1. **Reverse engineer** memory offsets with Cheat Engine
2. **Read** live values (health, armour, ammo) through static pointer chains
3. **Transmit** via ASCII serial protocol
4. **Display** on physical hardware driven by the Raspberry Pi Pico and a SSD1306



## Repo structure

```
./
├── src/
│   ├── main.c              # Entry point for the host application
│   ├── memory/             # Windows process memory API and offsets
│   └── telemetry/          # Windows serial telemetry transmission
├── pico/                   # Source files for the MCU and the display unit
│   └── telemetry_display/  # Includes data parsing, serial communication and display driver
├── docs/                   # Documents that follow the development of this project
└── VG_Telemetry.slnx       # Used to build the project
```

## Constraints

- **Read-only.** No `WriteProcessMemory`, no injection or process hooking.
- **Offline only.** Only to be used in single-player mode; no network interaction.
- **Pure C99.** No C++, no third-party libraries beyond the Windows API.

## Status

| Phase | State |
|---|---|
| RE & Memory Mapping | ✅ Health/Armor/Ammo offset verified |
| Host Application | ✅ Developed and tested on the host side |
| Serial Communication | ⏳ Testing pending |
| Pico Firmware | ⏳ Testing pending |


<p align="center">
  <i>Third live telemetry test — struct populated and then formatted with the telemetry functions</i>
</p>


https://github.com/user-attachments/assets/1b1096cb-afb0-4ecd-b5f1-cdabee5d1925




## License

This project is developed for academic purposes only. AssaultCube is an open-source game; no proprietary software is modified or redistributed.
