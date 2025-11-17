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

### Yazılım Gereksinimleri

-   **IDE:** Visual Studio Code
-   **Framework:** **ESP-IDF (Espressif IoT Development Framework)**. (Arduino DEĞİL!)
    -   VS Code için resmi "Espressif IDF" eklentisinin kurulması zorunludur.
-   **Dil:** C
-   **İşletim Sistemi:** FreeRTOS (ESP-IDF ile birlikte gelir)

## Geliştirme Yol Haritası

Bu branch'teki kod, nihai hedefin kavramsal bir temsilidir. Çalışır hale getirmek için aşağıdaki adımlar izlenmelidir:

1.  **Ortam Kurulumu:** Bilgisayarınıza VS Code ve ESP-IDF eklentisini kurun.
2.  **HFP Örneğini Test Etme:** ESP-IDF içindeki `hfp_hf_demo` örneğini çalıştırarak ses aktarımının temelini doğrulayın.
3.  **HID Örneğini Test Etme:** `ble_hid_device_demo` örneğini çalıştırarak BLE kontrolcü fonksiyonlarını doğrulayın.
4.  **I2S Mikrofonu Test Etme:** `i2s` örneklerinden biriyle mikrofondan ses okunduğunu doğrulayın.
5.  **Kodları Birleştirme:** Bu üç çalışan örneği, bu branch'teki `main.c` dosyasında bulunan mantığa göre adım adım birleştirin.

Mevcut kod, bu birleştirme işlemi için bir kılavuz ve iskelet görevi görmektedir.