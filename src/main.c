#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/i2s.h"

// Bluetooth Kütüphaneleri
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_hidd_prf_api.h"
#include "esp_hf_client_api.h"
#include "hid_dev.h"

static const char* TAG = "ULTIMATE_REMOTE";

// --- Pin Tanımlamaları ---
#define JOY_X_PIN   ADC1_CHANNEL_6 // GPIO 34
#define JOY_Y_PIN   ADC1_CHANNEL_7 // GPIO 35
#define JOY_SW_PIN  GPIO_NUM_32
#define BACK_PIN    GPIO_NUM_19
#define PLAY_PAUSE_PIN GPIO_NUM_18
#define ASSISTANT_PIN GPIO_NUM_5
#define VOL_UP_PIN       GPIO_NUM_17 // Changed from 14 to 17 to avoid conflict
#define VOL_DOWN_PIN     GPIO_NUM_16 // Changed from 12 to 16 to avoid conflict
#define NEXT_TRACK_PIN   GPIO_NUM_4  // Changed from 13 to 4 to avoid conflict
#define PREV_TRACK_PIN   GPIO_NUM_21 // Changed from 15 to 21 to avoid conflict

#define I2S_WS      GPIO_NUM_25
#define I2S_SCK     GPIO_NUM_26
#define I2S_SD      GPIO_NUM_22

// --- Prototip Fonksiyonlar ---
void hid_demo_task(void *pvParameters);
void handle_command(char command);
void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param); // Added prototype

// --- HFP ve HID Global Değişkenleri ---
static uint16_t hid_conn_id = 0;
static bool sec_conn = false;
static bool hfp_audio_connected = false;

// --- HFP Olay Yöneticisi (Callback) ---
void bt_app_hf_client_cb(esp_hf_client_cb_event_t event, esp_hf_client_cb_param_t *param)
{
    switch (event) {
        case ESP_HF_CLIENT_AUDIO_STATE_EVT:
            if (param->audio_state.state == ESP_HF_CLIENT_AUDIO_STATE_CONNECTED) {
                hfp_audio_connected = true;
                ESP_LOGI(TAG, "HFP Ses Bağlantısı KURULDU");
            } else if (param->audio_state.state == ESP_HF_CLIENT_AUDIO_STATE_DISCONNECTED) {
                hfp_audio_connected = false;
                ESP_LOGI(TAG, "HFP Ses Bağlantısı KESİLDİ");
            }
            break;
        // Diğer HFP event'leri (bağlantı, arama vb.) burada yönetilir...
        default:
            ESP_LOGI(TAG, "HFP Event: %d", event);
            break;
    }
}

// --- HID/GATT Olay Yöneticisi (Callback) ---
void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    switch(event) {
        case ESP_GATTS_CONNECT_EVT:
            hid_conn_id = param->connect.conn_id;
            ESP_LOGI(TAG, "HID Cihazı Bağlandı, conn_id = %d", hid_conn_id);
            sec_conn = true; // Assume secure connection for simplicity in this conceptual code
            break;
        case ESP_GATTS_DISCONNECT_EVT:
            sec_conn = false;
            hid_conn_id = 0;
            ESP_LOGI(TAG, "HID Cihazının Bağlantısı Kesildi");
            esp_ble_gap_start_advertising(NULL); // Reklamı yeniden başlat
            break;
        case ESP_GATTS_REG_EVT:
            ESP_LOGI(TAG, "GATTS_REG_EVT, status %d, gatts_if %d", param->reg_evt.status, gatts_if);
            if (param->reg_evt.status == ESP_GATT_OK) {
                // HID profilini başlat
                esp_hidd_profile_init();
            }
            break;
        // Diğer GATT event'leri
        default:
            break;
    }
}

// --- GAP Olay Yöneticisi (Callback) ---
// Bu fonksiyon, ble_hid_device_demo örneğinden alınmıştır.
void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    switch (event) {
        case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
            esp_ble_gap_start_advertising(NULL);
            break;
        case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
            esp_ble_gap_start_advertising(NULL);
            break;
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(TAG, "Advertising start failed");
            } else {
                ESP_LOGI(TAG, "Advertising start successfully");
            }
            break;
        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
            if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(TAG, "Advertising stop failed");
            } else {
                ESP_LOGI(TAG, "Advertising stop successfully");
            }
            break;
        default:
            break;
    }
}

// --- I2S Mikrofon Kurulum Fonksiyonu ---
void setup_i2s_microphone() {
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_RX,
        .sample_rate = 8000, // HFP genellikle 8kHz veya 16kHz kullanır
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 1024,
        .use_apll = false, // Use APLL for better audio quality if available
    };
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD
    };
    i2s_set_pin(I2S_NUM_0, &pin_config);
    ESP_LOGI(TAG, "I2S Mikrofon sürücüsü kuruldu.");
}

// --- Görev: Fiziksel Butonları ve Joystick'i Oku ---
void physical_inputs_task(void *pvParameters) {
    // Pinleri ayarla
    gpio_set_direction(JOY_SW_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(JOY_SW_PIN, GPIO_PULLUP_ONLY);

    gpio_set_direction(BACK_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BACK_PIN, GPIO_PULLUP_ONLY);

    gpio_set_direction(PLAY_PAUSE_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(PLAY_PAUSE_PIN, GPIO_PULLUP_ONLY);

    gpio_set_direction(ASSISTANT_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(ASSISTANT_PIN, GPIO_PULLUP_ONLY);

    gpio_set_direction(VOL_UP_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(VOL_UP_PIN, GPIO_PULLUP_ONLY);

    gpio_set_direction(VOL_DOWN_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(VOL_DOWN_PIN, GPIO_PULLUP_ONLY);

    gpio_set_direction(NEXT_TRACK_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(NEXT_TRACK_PIN, GPIO_PULLUP_ONLY);

    gpio_set_direction(PREV_TRACK_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(PREV_TRACK_PIN, GPIO_PULLUP_ONLY);

    // ADC for Joystick
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(JOY_X_PIN, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(JOY_Y_PIN, ADC_ATTEN_DB_11);

    while(1) {
        if (gpio_get_level(JOY_SW_PIN) == 0) { handle_command('t'); }
        if (gpio_get_level(BACK_PIN) == 0) { handle_command('b'); }
        if (gpio_get_level(PLAY_PAUSE_PIN) == 0) { handle_command('p'); }
        if (gpio_get_level(ASSISTANT_PIN) == 0) { handle_command('x'); }
        if (gpio_get_level(VOL_UP_PIN) == 0) { handle_command('y'); }
        if (gpio_get_level(VOL_DOWN_PIN) == 0) { handle_command('l'); }
        if (gpio_get_level(NEXT_TRACK_PIN) == 0) { handle_command('i'); }
        if (gpio_get_level(PREV_TRACK_PIN) == 0) { handle_command('g'); }
        
        // Joystick oku
        int joy_y = adc1_get_raw(JOY_Y_PIN);
        int joy_x = adc1_get_raw(JOY_X_PIN);

        if (joy_y < 1000)      { handle_command('w'); } // Up
        else if (joy_y > 3000) { handle_command('s'); } // Down
        
        if (joy_x < 1000) { handle_command('a'); } // Left
        else if (joy_x > 3000) { handle_command('d'); } // Right

        vTaskDelay(pdMS_TO_TICKS(100)); // Her 100ms'de bir kontrol (debounce dahil)
    }
}

// --- Görev: Mikrofondan Sesi Oku ve Gönder ---
void audio_stream_task(void *pvParameters) {
    size_t bytes_read;
    // Allocate buffer for I2S data
    uint8_t *i2s_read_buffer = (uint8_t *)calloc(1024, sizeof(uint8_t));
    if (i2s_read_buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate I2S read buffer");
        vTaskDelete(NULL);
    }

    while(1) {
        if (hfp_audio_connected) {
            // Read data from I2S microphone
            i2s_read(I2S_NUM_0, (void *)i2s_read_buffer, 1024, &bytes_read, portMAX_DELAY);
            // Send audio data via HFP
            esp_hf_client_outgoing_data(i2s_read_buffer, bytes_read);
        } else {
            vTaskDelay(pdMS_TO_TICKS(100)); // Wait if no audio connection
        }
    }
    free(i2s_read_buffer);
}

// --- Ana Eylem Fonksiyonu: Komutları HID'ye Çevir ---
void handle_command(char command) {
    if (!sec_conn) { 
        ESP_LOGW(TAG, "HID bağlantısı güvenli değil veya bağlı değil.");
        return; 
    }

    uint8_t key_v;
    switch(command) {
        case 'w': // D-PAD UP
            // Gamepad report for D-PAD UP
            esp_hidd_send_gamepad_report(hid_conn_id, 0x00, 0, 0, 0, 0); // HAT_UP
            vTaskDelay(pdMS_TO_TICKS(50));
            esp_hidd_send_gamepad_report(hid_conn_id, 0x08, 0, 0, 0, 0); // HAT_CENTERED
            break;
        case 's': // D-PAD DOWN
            // Gamepad report for D-PAD DOWN
            esp_hidd_send_gamepad_report(hid_conn_id, 0x04, 0, 0, 0, 0); // HAT_DOWN
            vTaskDelay(pdMS_TO_TICKS(50));
            esp_hidd_send_gamepad_report(hid_conn_id, 0x08, 0, 0, 0, 0); // HAT_CENTERED
            break;
        case 'a': // D-PAD LEFT
            // Gamepad report for D-PAD LEFT
            esp_hidd_send_gamepad_report(hid_conn_id, 0x06, 0, 0, 0, 0); // HAT_LEFT
            vTaskDelay(pdMS_TO_TICKS(50));
            esp_hidd_send_gamepad_report(hid_conn_id, 0x08, 0, 0, 0, 0); // HAT_CENTERED
            break;
        case 'd': // D-PAD RIGHT
            // Gamepad report for D-PAD RIGHT
            esp_hidd_send_gamepad_report(hid_conn_id, 0x02, 0, 0, 0, 0); // HAT_RIGHT
            vTaskDelay(pdMS_TO_TICKS(50));
            esp_hidd_send_gamepad_report(hid_conn_id, 0x08, 0, 0, 0, 0); // HAT_CENTERED
            break;
        case 't': // ENTER/SELECT (Keyboard Return)
            esp_hidd_send_keyboard_value(hid_conn_id, HID_KEY_RETURN, true);
            vTaskDelay(pdMS_TO_TICKS(50));
            esp_hidd_send_keyboard_value(hid_conn_id, HID_KEY_RETURN, false);
            break;
        case 'p': // PLAY/PAUSE
            key_v = HID_CONSUMER_PLAY_PAUSE;
            esp_hidd_send_consumer_key(hid_conn_id, key_v, true);
            vTaskDelay(pdMS_TO_TICKS(50));
            esp_hidd_send_consumer_key(hid_conn_id, key_v, false);
            break;
        case 'y': // VOLUME UP
            key_v = HID_CONSUMER_VOLUME_INCREMENT;
            esp_hidd_send_consumer_key(hid_conn_id, key_v, true);
            vTaskDelay(pdMS_TO_TICKS(50));
            esp_hidd_send_consumer_key(hid_conn_id, key_v, false);
            break;
        case 'l': // VOLUME DOWN
            key_v = HID_CONSUMER_VOLUME_DECREMENT;
            esp_hidd_send_consumer_key(hid_conn_id, key_v, true);
            vTaskDelay(pdMS_TO_TICKS(50));
            esp_hidd_send_consumer_key(hid_conn_id, key_v, false);
            break;
        case 'i': // NEXT TRACK
            key_v = HID_CONSUMER_SCAN_NEXT_TRACK;
            esp_hidd_send_consumer_key(hid_conn_id, key_v, true);
            vTaskDelay(pdMS_TO_TICKS(50));
            esp_hidd_send_consumer_key(hid_conn_id, key_v, false);
            break;
        case 'g': // PREVIOUS TRACK
            key_v = HID_CONSUMER_SCAN_PREVIOUS_TRACK;
            esp_hidd_send_consumer_key(hid_conn_id, key_v, true);
            vTaskDelay(pdMS_TO_TICKS(50));
            esp_hidd_send_consumer_key(hid_conn_id, key_v, false);
            break;
        case 'b': // BACK (WWW_BACK)
            key_v = HID_CONSUMER_WWW_BACK;
            esp_hidd_send_consumer_key(hid_conn_id, key_v, true);
            vTaskDelay(pdMS_TO_TICKS(50));
            esp_hidd_send_consumer_key(hid_conn_id, key_v, false);
            break;
        case 'x': // ASSISTANT (LEFT_GUI)
            esp_hidd_send_keyboard_value(hid_conn_id, HID_KEY_LEFT_GUI, true);
            vTaskDelay(pdMS_TO_TICKS(50));
            esp_hidd_send_keyboard_value(hid_conn_id, HID_KEY_LEFT_GUI, false);
            break;
        default:
            ESP_LOGW(TAG, "Bilinmeyen komut: %c", command);
            break;
    }
}

// --- HID Demo Task (BLE HID Servisini başlatır) ---
// Bu fonksiyon, ESP-IDF'in "ble_hid_device_demo" örneğinden alınmıştır.
// HID profilini kurar ve reklamı başlatır.
void hid_demo_task(void *pvParameters) {
    ESP_LOGI(TAG, "HID Demo Task Başlatılıyor.");

    // HID Servislerini kaydet
    esp_ble_gatts_register_callback(gatts_event_handler);
    esp_ble_gap_register_callback(gap_event_handler);

    // HID cihazını başlat
    esp_hidd_dev_init(hid_report_map, sizeof(hid_report_map));

    // Reklam verilerini ayarla (örnekten alınmıştır)
    esp_ble_adv_data_raw_t adv_data_raw = {
        .len = 30,
        .data = {
            0x02, 0x01, 0x06, // Flags
            0x03, 0x03, 0x12, 0x18, // 16-bit Service UUIDs (HID Service)
            0x0A, 0x09, 'E', 'S', 'P', '3', '2', ' ', 'H', 'I', 'D' // Device Name
        }
    };
    esp_ble_gap_config_adv_data_raw(&adv_data_raw);

    vTaskDelete(NULL); // Görev tamamlandıktan sonra kendini sil
}


void app_main(void)
{
    // 1. NVS Flash'ı başlat
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    // 2. Bluetooth Stack'ini Dual-Mode olarak başlat
    esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT); // Önce hafızayı serbest bırak
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_bt_controller_init(&bt_cfg);
    esp_bt_controller_enable(ESP_BT_MODE_BTDM); // BTDM = Bluetooth Dual Mode
    esp_bluedroid_init();
    esp_bluedroid_enable();
    
    // 3. HFP Client Servisini başlat
    esp_hf_client_register_callback(bt_app_hf_client_cb);
    esp_hf_client_init();

    // 4. BLE HID Servisini başlat (hid_demo_task içinden)
    xTaskCreate(hid_demo_task, "hid_task", 6 * 1024, NULL, 5, NULL);

    // 5. I2S Mikrofonu ayarla
    setup_i2s_microphone();

    // 6. Görevleri başlat
    xTaskCreate(physical_inputs_task, "input_task", 2048, NULL, 5, NULL);
    xTaskCreate(audio_stream_task, "audio_task", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "Ana Kurulum Tamamlandı.");
}
