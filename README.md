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

### Connection Diagram (Microphone-less Version)

This diagram illustrates the connections for the joystick and buttons to the ESP32. All buttons and joystick switch should be connected between the specified ESP32 GPIO pin and GND (Ground). The joystick's X and Y axis outputs are analog and should be connected to the specified ADC-enabled GPIO pins.

| Component           | Connection Type | ESP32 Pin | Notes                                     |
| :------------------ | :-------------- | :-------- | :---------------------------------------- |
| Joystick X-Axis     | Analog Input    | GPIO 34   | Connect to ADC1_CH6. Reads analog voltage. |
| Joystick Y-Axis     | Analog Input    | GPIO 35   | Connect to ADC1_CH7. Reads analog voltage. |
| Joystick Switch     | Digital Input   | GPIO 32   | Connect to GND when pressed. Pull-up enabled in code. |
| Back Button         | Digital Input   | GPIO 25   | Connect to GND when pressed. Pull-up enabled in code. |
| Play/Pause Button   | Digital Input   | GPIO 26   | Connect to GND when pressed. Pull-up enabled in code. |
| Assistant Button    | Digital Input   | GPIO 27   | Connect to GND when pressed. Pull-up enabled in code. |
| Volume Up Button    | Digital Input   | GPIO 14   | Connect to GND when pressed. Pull-up enabled in code. |
| Volume Down Button  | Digital Input   | GPIO 12   | Connect to GND when pressed. Pull-up enabled in code. |
| Next Track Button   | Digital Input   | GPIO 13   | Connect to GND when pressed. Pull-up enabled in code. |
| Previous Track Button| Digital Input   | GPIO 15   | Connect to GND when pressed. Pull-up enabled in code. |
| **Common GND**      | Power           | GND       | Connect all component grounds here.       |
| **Common 3.3V**     | Power           | 3.3V      | Power supply for joystick (if needed).    |

#### Optional: Battery Connection Schematic

For portable use, you can power the ESP32 with a LiPo battery and a charging/protection circuit.

-   **LiPo Battery:** Connect the positive terminal to the `BAT+` input of the TP4056 module. Connect the negative terminal to the `BAT-` input of the TP4056 module.
-   **TP4056 Charging Module:**
    *   `OUT+` from TP4056 to `VIN` (or 5V) of ESP32 development board (if it has an onboard 5V regulator).
    *   `OUT-` from TP4056 to `GND` of ESP32 development board.
    *   Connect a 5V USB power source to the micro-USB port of the TP4056 module for charging.
-   **Alternative (Direct 3.3V):** If your ESP32 board can be powered directly via 3.3V (e.g., ESP32-WROOM-32 module), you might use a dedicated 3.3V step-down regulator from the battery.
    *   `BAT+` to input of 3.3V regulator.
    *   Output of 3.3V regulator to `3.3V` pin of ESP32.
    *   `BAT-` to `GND` of ESP32.
    *   **Caution:** Ensure your 3.3V regulator can supply enough current and has proper heat dissipation. Always use a battery protection circuit (often integrated into TP4056 modules or separate BMS).

## Software Requirements & Dependencies

-   **IDE:** [PlatformIO](https://platformio.org/) is recommended for managing the project, dependencies, and uploading the firmware.
-   **Framework:** [Arduino](https://www.arduino.cc/)
-   **Libraries:**
    -   `ESP32-BLE-Combo`: A powerful library that enables the ESP32 to act as a composite BLE HID device (Keyboard + Mouse). This library is included in the `lib/` folder of this project.

## Python Simulator

A dedicated Python GUI simulator is available to test the ESP32's functionality without physical hardware, or to visualize audio input for the ESP-IDF microphone version.

-   **Repository:** [ESP-Car-Remote-Simulator](https://github.com/alazndy/ESP-Car-Remote-Simulator.git)
-   **Location in this branch:** `esp32_controller_gui.py`

Refer to the simulator's repository for detailed setup and usage instructions.

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
