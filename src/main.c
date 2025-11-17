#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include "esp_log.h"

static const char* TAG = "MIC_STREAMER";

// --- I2S Mikrofon Ayarları ---
// Bu ayarların Python tarafındaki ayarlarla AYNI olması KRİTİKTİR!
#define I2S_PORT_NUM      I2S_NUM_0
#define I2S_SAMPLE_RATE   (16000) // 16kHz, sesli komutlar için iyi bir kalite
#define I2S_SAMPLE_BITS   I2S_BITS_PER_SAMPLE_16BIT
#define I2S_CHANNEL_FMT   I2S_CHANNEL_FMT_ONLY_LEFT
#define I2S_READ_BUF_SIZE (2048)

// --- Pin Tanımlamaları ---
#define I2S_SCK_PIN     GPIO_NUM_26
#define I2S_WS_PIN      GPIO_NUM_25
#define I2S_SD_PIN      GPIO_NUM_22

void i2s_mic_init() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_SAMPLE_BITS,
        .channel_format = I2S_CHANNEL_FMT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 1024,
    };
    i2s_driver_install(I2S_PORT_NUM, &i2s_config, 0, NULL);

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK_PIN,
        .ws_io_num = I2S_WS_PIN,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD_PIN
    };
    i2s_set_pin(I2S_PORT_NUM, &pin_config);
    ESP_LOGI(TAG, "I2S Mikrofon sürücüsü kuruldu.");
}

void audio_stream_task(void *arg) {
    size_t bytes_read;
    char* read_buf = (char*) calloc(I2S_READ_BUF_SIZE, sizeof(char));

    while (1) {
        // Mikrofondan ham ses verisini oku
        i2s_read(I2S_PORT_NUM, read_buf, I2S_READ_BUF_SIZE, &bytes_read, portMAX_DELAY);
        
        // Okunan veriyi doğrudan seri porta yazdır
        // printf kullanmıyoruz çünkü o formatlama yapar. Ham veri gönderiyoruz.
        fwrite(read_buf, 1, bytes_read, stdout);
        fflush(stdout); // Verinin hemen gönderildiğinden emin ol
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "Mikrofon Ses Yayıncısı Başlatılıyor...");
    i2s_mic_init();
    xTaskCreate(audio_stream_task, "Audio Stream Task", 4096, NULL, 5, NULL);
}