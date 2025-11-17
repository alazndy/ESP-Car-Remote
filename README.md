# ESP32 Arduino Controller - BLE HID Remote

This branch contains the **Arduino framework** implementation of the ESP32 remote control. It focuses on providing basic Bluetooth Low Energy (BLE) Human Interface Device (HID) functionality, acting as a keyboard and mouse for media control and navigation. This version is designed to be simpler to set up and program using PlatformIO with the Arduino framework.

## Features

-   **Bluetooth Low Energy (BLE) HID:** Connects as a standard BLE keyboard and mouse.
-   **Media Controls:** Play/Pause, Volume Up/Down, Next/Previous Track.
-   **System Navigation:** Directional controls (Up, Down, Left, Right), Enter/Select, Back.
-   **Assistant Integration:** A dedicated button to activate the connected device's voice assistant.
-   **Dual Mode Operation:** Supports both a simulator mode (via Serial Monitor) and physical hardware input.

## Hardware Requirements

For **Physical Mode**, you will need the following components:

-   **Microcontroller:** ESP32 Dev Kit (or any similar ESP32 development board)
-   **Input Controls:**
    -   1 x Analog Joystick Module (2-axis with push-button)
    -   7 x Tactile Push Buttons
-   **Wiring:** Jumper wires and a breadboard or a custom PCB.

### Pinout

| Component           | ESP32 Pin |
| ------------------- | --------- |
| Joystick X-Axis     | 34        |
| Joystick Y-Axis     | 35        |
| Joystick Switch     | 32        |
| Back Button         | 25        |
| Play/Pause Button   | 26        |
| Assistant Button    | 27        |
| Volume Up Button    | 14        |
| Volume Down Button  | 12        |
| Next Track Button   | 13        |
| Previous Track Button| 15        |

## Software Requirements & Dependencies

-   **IDE:** [PlatformIO](https://platformio.org/) is recommended for managing the project, dependencies, and uploading the firmware.
-   **Framework:** [Arduino](https://www.arduino.cc/)
-   **Libraries:**
    -   `ESP32-BLE-Combo`: A powerful library that enables the ESP32 to act as a composite BLE HID device (Keyboard + Mouse). This library is included in the `lib/` folder of this project.

## Installation & Setup

1.  **Clone this branch:**
    ```bash
    git clone -b arduino_controller https://github.com/alazndy/ESP-Car-Remote.git
    ```
2.  **Open in PlatformIO:**
    -   Open Visual Studio Code with the PlatformIO extension installed.
    -   Click on the PlatformIO icon in the sidebar.
    -   Click "Open Project" and navigate to the cloned `ESP-Car-Remote` folder.
3.  **Configure the Mode:**
    -   Open the `src/main.cpp` file.
    -   To use with physical hardware, set `SIMULATOR_MODE` to `false`.
    -   To test using the serial monitor, set `SIMULATOR_MODE` to `true`.
    ```cpp
    // Set to 'true' to test with the Serial Simulator
    // Set to 'false' for physical joystick and buttons
    #define SIMULATOR_MODE false
    ```
4.  **Build and Upload:**
    -   Connect your ESP32 board to your computer.
    -   In PlatformIO, click the "Upload" button (the arrow icon) in the bottom status bar.

## Usage

### Physical Mode

Once the firmware is uploaded and the hardware is wired according to the pinout table, the device is ready to use.

1.  Power on the ESP32.
2.  On your car's infotainment system (or any other device like a phone or tablet), scan for Bluetooth devices.
3.  Connect to the device named **"ESP32 Medya & Sistem"**.
4.  You can now use the joystick and buttons to control your system.

### Simulator Mode

1.  Ensure `SIMULATOR_MODE` is set to `true` and upload the firmware.
2.  Open the PlatformIO Serial Monitor (the plug icon in the status bar).
3.  Connect to the device as described above.
4.  Type the following single-character commands into the serial monitor and press Enter to simulate button presses:

| Command | Action              |
| ------- | ------------------- |
| `w`     | Navigate Up         |
| `s`     | Navigate Down       |
| `a`     | Navigate Left       |
| `d`     | Navigate Right      |
| `t`     | Enter/Select        |
| `p`     | Play/Pause          |
| `y`     | Volume Up           |
| `l`     | Volume Down         |
| `i`     | Next Track          |
| `g`     | Previous Track      |
| `b`     | Back                |
| `x`     | Activate Assistant  |

## License

This project is distributed under the MIT License.
