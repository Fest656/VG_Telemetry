# Setup & Installation Guide

## 1. Prerequisites

### Host Application (Windows)
* **AssaultCube v1.3.0.2**: The exact version is required because newer version might have altered offsets.
* **Visual Studio 2022**: With the "Desktop development with C++" workload installed.
* 
### Pico Firmware
* **Raspberry Pi Pico C/C++ SDK**: The official toolchain.
* **VS Code**: With the official Raspberry Pi Pico extension installed (this manages CMake and the SDK for you).

---

## 2. Getting the Repository
Clone the repository to your local machine:
```bash
git clone https://github.com/Fest656/VG_Telemetry.git
cd VG_Telemetry
```

---

## 3. Compiling the Project

### Part A: The Host Application
1. Navigate to the root directory of the cloned repository.
2. Open `config/config.h` and set `BUILD_MODE` to the desired mode (`E2E`, `PICO_ECHO`, or `CONSOLE`). See the [README](../README.md#build-modes) for details on each mode.
3. Open `VG_Telemetry.sln` in Visual Studio.
4. Set the build configuration to **Release** and architecture to **x64**.
5. Go to `Build > Build Solution` (or press `Ctrl+Shift+B`).
6. The compiled executable (`VG_Telemetry.exe`) will be generated in the respective output folder `bin/`.

### Part B: The Pico Firmware
1. Open the `firmware/telemetry_display` folder in Visual Studio Code.
2. Ensure the Raspberry Pi Pico extension is active and the SDK is configured.
3. Click the **Compile Project** button in the Pico extension sidebar (or use the CMake targets on the bottom toolbar).
4. Once compiled, locate the generated `.uf2` file in the `build/` directory.
5. Hold the **BOOTSEL** button on your Raspberry Pi Pico and while holding it, plug the Pico into your PC via Micro-USB.
6. Drag and drop the `.uf2` file into the `RPI-RP2` storage drive that appears. The Pico will reboot automatically and run the firmware.

> [!Note]
> Once the Raspberry Pi Pico is plugged in and running the firmware, you can open your device manager and expand tyhe `Ports` category to see what new COM ports appear, that will be the one you enter in the host application.

---

## 4. Running the Telemetry Tracker

> [!WARNING]
> You must start the host application while sitting in the **base lobby** of AssaultCube.

1. Launch **AssaultCube v1.3.0.2**.
2. At this point everything should be wired, plugged in and running the firmware.
3. Run `VG_Telemetry.exe` (run as Administrator if you encounter memory reading access issues, though standard privileges usually suffice for `ReadProcessMemory`). If running in `E2E` or `PICO_ECHO` mode, the application will prompt you to enter a COM port (e.g. `COM3`).
4. The console should confirm that it found the `ac_client.exe` process and successfully read the local player pointer.
5. Start a single-player match. Your live health, armor, ammo, kills, and deaths should now stream perfectly to the OLED display.
