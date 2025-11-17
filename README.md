# ESP32 Ultimate Remote - EXPERIMENTAL BRANCH

**DİKKAT:** Bu branch, projenin Arduino altyapısından **ESP-IDF (Espressif IoT Development Framework)** altyapısına taşındığı ve yeni özelliklerin eklendiği geliştirme sürümünü içermektedir. Bu koddaki yapılar, ana branch'teki Arduino kodundan tamamen farklıdır ve çalışması için profesyonel bir geliştirme ortamı gerektirir.

## Projenin Amacı (Bu Branch için)

Bu branch'in temel hedefi, ESP32'nin tüm yeteneklerini kullanarak standart bir BLE kumandanın ötesine geçmektir. Amaç, aşağıdaki iki ana özelliği aynı anda çalıştıran **hibrit bir akıllı kumanda** oluşturmaktır:

1.  **Bluetooth Classic (HFP - Hands-Free Profile):** Cihaza bir I2S mikrofon (INMP441) ekleyerek, ESP32'nin bir Bluetooth kulaklık gibi davranmasını sağlamak. Bu, telefon görüşmeleri yapmak veya sesli asistanlara komut göndermek için ses aktarımına olanak tanır.
2.  **Bluetooth Low Energy (BLE - HID Profili):** Cihazın aynı anda bir klavye ve gamepad olarak tanınmasını sağlamak. Bu, medya kontrolü (ses, şarkı değiştirme) ve menülerde D-Pad ile gezinme gibi işlevleri yönetir.

Kısacası, bu branch projemizi "eller serbest" ses özelliklerine sahip, tam fonksiyonlu bir akıllı araba kumandasına dönüştürmeyi hedefler.

## Gereksinimler

### Donanım Gereksinimleri

-   **Mikrodenetleyici:** ESP32 Geliştirme Kartı
-   **Mikrofon:** **INMP441** I2S Dijital Mikrofon Modülü (Ses kalitesi için kritik)
-   **Kontrol:** Analog Joystick Modülü ve Fiziksel Butonlar
-   **Güç:** Harici bir güç devresi (Pil, TP4056 şarj modülü, 3.3V regülatör) tavsiye edilir.

### Connection Diagram (Microphone-enabled Version)

This diagram illustrates the connections for the joystick, buttons, and the INMP441 I2S microphone to the ESP32. All buttons and joystick switch should be connected between the specified ESP32 GPIO pin and GND (Ground). The joystick's X and Y axis outputs are analog and should be connected to the specified ADC-enabled GPIO pins. The I2S microphone requires specific I2S pins.

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
| **INMP441 I2S Mic** |                 |           |                                           |
| ├── SCK             | Digital Output  | GPIO 26   | I2S Clock.                                |
| ├── WS              | Digital Output  | GPIO 25   | I2S Word Select (Left/Right Channel).     |
| └── SD              | Digital Input   | GPIO 22   | I2S Serial Data.                          |
| **Common GND**      | Power           | GND       | Connect all component grounds here.       |
| **Common 3.3V**     | Power           | 3.3V      | Power supply for joystick and microphone. |

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

### Yazılım Gereksinimleri

-   **IDE:** Visual Studio Code
-   **Framework:** **ESP-IDF (Espressif IoT Development Framework)**. (Arduino DEĞİL!)
    -   VS Code için resmi "Espressif IDF" eklentisinin kurulması zorunludur.
-   **Dil:** C
-   **İşletim Sistemi:** FreeRTOS (ESP-IDF ile birlikte gelir)

## Python Simulator

A dedicated Python GUI simulator is available to test the ESP32's functionality, especially for visualizing audio input from the I2S microphone.

-   **Repository:** [ESP-Car-Remote-Simulator](https://github.com/alazndy/ESP-Car-Remote-Simulator.git)
-   **Location in this branch:** `esp32_controller_gui.py`

Refer to the simulator's repository for detailed setup and usage instructions.

## Geliştirme Yol Haritası

Bu branch'teki kod, nihai hedefin kavramsal bir temsilidir. Çalışır hale getirmek için aşağıdaki adımlar izlenmelidir:

1.  **Ortam Kurulumu:** Bilgisayarınıza VS Code ve ESP-IDF eklentisini kurun.
2.  **HFP Örneğini Test Etme:** ESP-IDF içindeki `hfp_hf_demo` örneğini çalıştırarak ses aktarımının temelini doğrulayın.
3.  **HID Örneğini Test Etme:** `ble_hid_device_demo` örneğini çalıştırarak BLE kontrolcü fonksiyonlarını doğrulayın.
4.  **I2S Mikrofonu Test Etme:** `i2s` örneklerinden biriyle mikrofondan ses okunduğunu doğrulayın.
5.  **Kodları Birleştirme:** Bu üç çalışan örneği, bu branch'teki `main.c` dosyasında bulunan mantığa göre adım adım birleştirin.

Mevcut kod, bu birleştirme işlemi için bir kılavuz ve iskelet görevi görmektedir.