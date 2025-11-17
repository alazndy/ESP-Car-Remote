# ESP32 Ultimate Remote - Hybrid Bluetooth Controller

This repository hosts the development of a versatile ESP32-based Bluetooth remote control, designed to function as a hybrid controller for various applications, primarily focusing on car multimedia and smart assistant integration. The project explores both Arduino and ESP-IDF frameworks to achieve different levels of functionality.

## Project Overview

The core idea is to transform an ESP32 into a powerful input device capable of:
- Emulating a standard Human Interface Device (HID) for keyboard and gamepad controls.
- Integrating advanced audio capabilities, such as a microphone for hands-free communication or voice assistant commands.

Due to the complexity of combining Bluetooth Low Energy (BLE) HID with Bluetooth Classic Hands-Free Profile (HFP) for audio, the project is structured into different branches, each focusing on a specific implementation or feature set.

## Branches

This repository is organized into the following main branches:

-   **`main`**: This branch serves as the stable entry point for the project. It provides an overview of the different development paths and links to the specific feature branches. It does not contain directly runnable code for a specific controller version but acts as a central hub.

-   **`arduino_controller`**: This branch contains the **Arduino framework** implementation of the remote control. It focuses on basic BLE HID functionality, acting as a keyboard and mouse for media control and navigation. This version is simpler to set up and program using PlatformIO with the Arduino framework.
    *   **Features:** BLE HID (Keyboard, Mouse), Media Controls, Navigation, Simulator/Physical modes.
    *   **Target Audience:** Users looking for a straightforward BLE remote without microphone integration.

-   **`esp_idf_microphone`**: This branch represents the advanced **ESP-IDF framework** implementation, which includes **microphone integration** via Bluetooth Classic's Hands-Free Profile (HFP). This version is significantly more complex, requiring a deeper understanding of ESP-IDF and C programming, but offers dual-mode Bluetooth capabilities (BLE HID + BT Classic HFP).
    *   **Features:** Bluetooth Dual-Mode (BLE HID + BT Classic HFP), Microphone Input, Gamepad/Keyboard Controls, Advanced ESP-IDF architecture.
    *   **Target Audience:** Developers aiming for a full-featured smart assistant or hands-free communication device.

## Getting Started

To explore a specific version of the project, please checkout the respective branch:

```bash
# For the Arduino-based controller
git checkout arduino_controller

# For the ESP-IDF based microphone-enabled controller
git checkout esp_idf_microphone
```

Each branch contains its own detailed `README.md` with specific setup, hardware, software, and usage instructions.

## Contributing

Contributions are welcome! Please refer to the `CONTRIBUTING.md` (if available) or the `README.md` of the specific branch you wish to contribute to for guidelines.

## License

This project is distributed under the MIT License. See `LICENSE` for more information.