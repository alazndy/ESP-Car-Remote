// --- YENİ BleComboKeyboard.cpp DOSYASININ İÇERİĞİ ---

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "BLE2902.h"
#include "BLEHIDDevice.h"
#include "HIDTypes.h"
#include <driver/adc.h>
#include "sdkconfig.h"

#include "BleConnectionStatus.h"
#include "KeyboardOutputCallbacks.h"
#include "BleComboKeyboard.h"

#if defined(CONFIG_ARDUHAL_ESP_LOG)
  #include "esp32-hal-log.h"
  #define LOG_TAG ""
#else
  #include "esp_log.h"
  static const char* LOG_TAG = "BLEDevice";
#endif


// Report IDs:
#define KEYBOARD_ID 0x01
#define MEDIA_KEYS_ID 0x02
#define MOUSE_ID 0x03
#define CONSUMER_KEYS_ID 0x04 // YENİ: Güç tuşu için Rapor ID'si

static const uint8_t _hidReportDescriptor[] = {
  USAGE_PAGE(1),      0x01, USAGE(1),           0x06, COLLECTION(1),      0x01,
  REPORT_ID(1),       KEYBOARD_ID, USAGE_PAGE(1),      0x07, USAGE_MINIMUM(1),   0xE0, USAGE_MAXIMUM(1),   0xE7,
  LOGICAL_MINIMUM(1), 0x00, LOGICAL_MAXIMUM(1), 0x01, REPORT_SIZE(1),     0x01, REPORT_COUNT(1),    0x08,
  HIDINPUT(1),        0x02, REPORT_COUNT(1),    0x01, REPORT_SIZE(1),     0x08, HIDINPUT(1),        0x01,
  REPORT_COUNT(1),    0x05, REPORT_SIZE(1),     0x01, USAGE_PAGE(1),      0x08, USAGE_MINIMUM(1),   0x01,
  USAGE_MAXIMUM(1),   0x05, HIDOUTPUT(1),       0x02, REPORT_COUNT(1),    0x01, REPORT_SIZE(1),     0x03,
  HIDOUTPUT(1),       0x01, REPORT_COUNT(1),    0x06, REPORT_SIZE(1),     0x08, LOGICAL_MINIMUM(1), 0x00,
  LOGICAL_MAXIMUM(1), 0x65, USAGE_PAGE(1),      0x07, USAGE_MINIMUM(1),   0x00, USAGE_MAXIMUM(1),   0x65,
  HIDINPUT(1),        0x00, END_COLLECTION(0),
  
  USAGE_PAGE(1),      0x0C, USAGE(1),           0x01, COLLECTION(1),      0x01,
  REPORT_ID(1),       MEDIA_KEYS_ID, USAGE_PAGE(1),      0x0C, LOGICAL_MINIMUM(1), 0x00,
  LOGICAL_MAXIMUM(1), 0x01, REPORT_SIZE(1),     0x01, REPORT_COUNT(1),    0x10, USAGE(1),           0xB5,
  USAGE(1),           0xB6, USAGE(1),           0xB7, USAGE(1),           0xCD, USAGE(1),           0xE2,
  USAGE(1),           0xE9, USAGE(1),           0xEA, USAGE(2),           0x23, 0x02, USAGE(2),           0x94, 0x01,
  USAGE(2),           0x92, 0x01, USAGE(2),           0x2A, 0x02, USAGE(2),           0x21, 0x02, USAGE(2),           0x26, 0x02,
  USAGE(2),           0x24, 0x02, USAGE(2),           0x83, 0x01, USAGE(2),           0x8A, 0x01, HIDINPUT(1),        0x02,
  END_COLLECTION(0),

  // YENİ: Güç tuşu gibi Sistem Kontrol komutları için eklendi
  USAGE_PAGE(1),      0x01,       // USAGE_PAGE (Generic Desktop)
  USAGE(1),           0x80,       // USAGE (System Control)
  COLLECTION(1),      0x01,       // COLLECTION (Application)
  REPORT_ID(1),       CONSUMER_KEYS_ID, //   REPORT_ID (4)
  LOGICAL_MINIMUM(1), 0x00,       //   LOGICAL_MINIMUM (0)
  LOGICAL_MAXIMUM(2), 0xFF, 0x00, //   LOGICAL_MAXIMUM (255)
  USAGE_MINIMUM(1),   0x00,       //   USAGE_MINIMUM (Undefined)
  USAGE_MAXIMUM(1),   0xB7,       //   USAGE_MAXIMUM (System Menu Down)
  REPORT_SIZE(1),     0x08,       //   REPORT_SIZE (8)
  REPORT_COUNT(1),    0x01,       //   REPORT_COUNT (1)
  HIDINPUT(1),        0x00,       //   INPUT (Data,Array,Abs)
  END_COLLECTION(0),

  USAGE_PAGE(1),       0x01, USAGE(1),            0x02, COLLECTION(1),       0x01,
  USAGE(1),            0x01, COLLECTION(1),       0x00, REPORT_ID(1),        MOUSE_ID,
  USAGE_PAGE(1),       0x09, USAGE_MINIMUM(1),    0x01, USAGE_MAXIMUM(1),    0x05,
  LOGICAL_MINIMUM(1),  0x00, LOGICAL_MAXIMUM(1),  0x01, REPORT_SIZE(1),      0x01,
  REPORT_COUNT(1),     0x05, HIDINPUT(1),         0x02, REPORT_SIZE(1),      0x03,
  REPORT_COUNT(1),     0x01, HIDINPUT(1),         0x03, USAGE_PAGE(1),       0x01,
  USAGE(1),            0x30, USAGE(1),            0x31, USAGE(1),            0x38,
  LOGICAL_MINIMUM(1),  0x81, LOGICAL_MAXIMUM(1),  0x7f, REPORT_SIZE(1),      0x08,
  REPORT_COUNT(1),     0x03, HIDINPUT(1),         0x06, USAGE_PAGE(1),       0x0c,
  USAGE(2),      0x38, 0x02, LOGICAL_MINIMUM(1),  0x81, LOGICAL_MAXIMUM(1),  0x7f,
  REPORT_SIZE(1),      0x08, REPORT_COUNT(1),     0x01, HIDINPUT(1),         0x06,
  END_COLLECTION(0), END_COLLECTION(0)
};

BleComboKeyboard::BleComboKeyboard(std::string deviceName, std::string deviceManufacturer, uint8_t batteryLevel) : hid(0)
{
  this->deviceName = deviceName;
  this->deviceManufacturer = deviceManufacturer;
  this->batteryLevel = batteryLevel;
  this->connectionStatus = new BleConnectionStatus();
}

void BleComboKeyboard::begin(void)
{
  xTaskCreate(this->taskServer, "server", 20000, (void *)this, 5, NULL);
}

void BleComboKeyboard::end(void) { }

bool BleComboKeyboard::isConnected(void) {
  return this->connectionStatus->connected;
}

void BleComboKeyboard::setBatteryLevel(uint8_t level) {
  this->batteryLevel = level;
  if (hid != 0)
    this->hid->setBatteryLevel(this->batteryLevel);
}

void BleComboKeyboard::taskServer(void* pvParameter) {
  BleComboKeyboard* bleKeyboardInstance = (BleComboKeyboard *) pvParameter;
  BLEDevice::init(bleKeyboardInstance->deviceName);
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(bleKeyboardInstance->connectionStatus);

  bleKeyboardInstance->hid = new BLEHIDDevice(pServer);
  bleKeyboardInstance->inputKeyboard = bleKeyboardInstance->hid->inputReport(KEYBOARD_ID);
  bleKeyboardInstance->outputKeyboard = bleKeyboardInstance->hid->outputReport(KEYBOARD_ID);
  bleKeyboardInstance->inputMediaKeys = bleKeyboardInstance->hid->inputReport(MEDIA_KEYS_ID);
  bleKeyboardInstance->inputConsumerKeys = bleKeyboardInstance->hid->inputReport(CONSUMER_KEYS_ID); // YENİ
  
  bleKeyboardInstance->connectionStatus->inputKeyboard = bleKeyboardInstance->inputKeyboard;
  bleKeyboardInstance->connectionStatus->outputKeyboard = bleKeyboardInstance->outputKeyboard;
  
  bleKeyboardInstance->inputMouse = bleKeyboardInstance->hid->inputReport(MOUSE_ID);
  bleKeyboardInstance->connectionStatus->inputMouse = bleKeyboardInstance->inputMouse;
 
  bleKeyboardInstance->outputKeyboard->setCallbacks(new KeyboardOutputCallbacks());

  bleKeyboardInstance->hid->manufacturer()->setValue(bleKeyboardInstance->deviceManufacturer);

  bleKeyboardInstance->hid->pnp(0x02, 0xe502, 0xa111, 0x0210);
  bleKeyboardInstance->hid->hidInfo(0x00,0x01);

  BLESecurity *pSecurity = new BLESecurity();
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);

  bleKeyboardInstance->hid->reportMap((uint8_t*)_hidReportDescriptor, sizeof(_hidReportDescriptor));
  bleKeyboardInstance->hid->startServices();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->setAppearance(HID_KEYBOARD);
  pAdvertising->addServiceUUID(bleKeyboardInstance->hid->hidService()->getUUID());
  pAdvertising->start();
  bleKeyboardInstance->hid->setBatteryLevel(bleKeyboardInstance->batteryLevel);

  ESP_LOGD(LOG_TAG, "Advertising started!");
  vTaskDelay(portMAX_DELAY);
}

void BleComboKeyboard::sendReport(KeyReport* keys) {
  if (this->isConnected()) {
    this->inputKeyboard->setValue((uint8_t*)keys, sizeof(KeyReport));
    this->inputKeyboard->notify();
  }
}

void BleComboKeyboard::sendReport(MediaKeyReport* keys) {
  if (this->isConnected()) {
    this->inputMediaKeys->setValue((uint8_t*)keys, sizeof(MediaKeyReport));
    this->inputMediaKeys->notify();
  }
}

// YENİ: Sistem tuş raporunu gönderen fonksiyon
void BleComboKeyboard::sendReport(ConsumerKeyReport* key) {
  if (this->isConnected()) {
    this->inputConsumerKeys->setValue((uint8_t*)key, sizeof(ConsumerKeyReport));
    this->inputConsumerKeys->notify();
  }
}

extern const uint8_t _asciimap[128] PROGMEM;
#define SHIFT 0x80
const uint8_t _asciimap[128] = { /* ... Sizin verdiğiniz dosyadaki _asciimap içeriği ... */ 
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2a,0x2b,0x28,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x2c,0x1e|SHIFT,0x34|SHIFT,0x20|SHIFT,0x21|SHIFT,0x22|SHIFT,0x24|SHIFT,0x34,
	0x26|SHIFT,0x27|SHIFT,0x25|SHIFT,0x2e|SHIFT,0x36,0x2d,0x37,0x38,0x27,0x1e,0x1f,
	0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x33|SHIFT,0x33,0x36|SHIFT,0x2e,0x37|SHIFT,
	0x38|SHIFT,0x1f|SHIFT,0x04|SHIFT,0x05|SHIFT,0x06|SHIFT,0x07|SHIFT,0x08|SHIFT,
	0x09|SHIFT,0x0a|SHIFT,0x0b|SHIFT,0x0c|SHIFT,0x0d|SHIFT,0x0e|SHIFT,0x0f|SHIFT,
	0x10|SHIFT,0x11|SHIFT,0x12|SHIFT,0x13|SHIFT,0x14|SHIFT,0x15|SHIFT,0x16|SHIFT,
	0x17|SHIFT,0x18|SHIFT,0x19|SHIFT,0x1a|SHIFT,0x1b|SHIFT,0x1c|SHIFT,0x1d|SHIFT,
	0x2f,0x31,0x30,0x23|SHIFT,0x2d|SHIFT,0x35,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,
	0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,
	0x1b,0x1c,0x1d,0x2f|SHIFT,0x31|SHIFT,0x30|SHIFT,0x35|SHIFT,0
};

size_t BleComboKeyboard::press(uint8_t k) { /* ... Sizin verdiğiniz dosyadaki press fonksiyonu ... */ 
	uint8_t i; if (k >= 136) { k = k - 136; } else if (k >= 128) { _keyReport.modifiers |= (1<<(k-128)); k = 0; } else { k = pgm_read_byte(_asciimap + k); if (!k) { setWriteError(); return 0; } if (k & 0x80) { _keyReport.modifiers |= 0x02;	k &= 0x7F; } } if (_keyReport.keys[0] != k && _keyReport.keys[1] != k && _keyReport.keys[2] != k && _keyReport.keys[3] != k && _keyReport.keys[4] != k && _keyReport.keys[5] != k) { for (i=0; i<6; i++) { if (_keyReport.keys[i] == 0x00) { _keyReport.keys[i] = k; break; } } if (i == 6) { setWriteError(); return 0; } } sendReport(&_keyReport); return 1;
}

size_t BleComboKeyboard::press(const MediaKeyReport k) { /* ... Sizin verdiğiniz dosyadaki press fonksiyonu ... */
    uint16_t k_16 = k[1] | (k[0] << 8); uint16_t mediaKeyReport_16 = _mediaKeyReport[1] | (_mediaKeyReport[0] << 8); mediaKeyReport_16 |= k_16; _mediaKeyReport[0] = (uint8_t)((mediaKeyReport_16 & 0xFF00) >> 8); _mediaKeyReport[1] = (uint8_t)(mediaKeyReport_16 & 0x00FF); sendReport(&_mediaKeyReport); return 1;
}

size_t BleComboKeyboard::release(uint8_t k) { /* ... Sizin verdiğiniz dosyadaki release fonksiyonu ... */
	uint8_t i; if (k >= 136) { k = k - 136; } else if (k >= 128) {	_keyReport.modifiers &= ~(1<<(k-128)); k = 0; } else { k = pgm_read_byte(_asciimap + k); if (!k) { return 0; } if (k & 0x80) { _keyReport.modifiers &= ~(0x02); k &= 0x7F; } } for (i=0; i<6; i++) { if (0 != k && _keyReport.keys[i] == k) { _keyReport.keys[i] = 0x00; } } sendReport(&_keyReport); return 1;
}

size_t BleComboKeyboard::release(const MediaKeyReport k) { /* ... Sizin verdiğiniz dosyadaki release fonksiyonu ... */
    uint16_t k_16 = k[1] | (k[0] << 8); uint16_t mediaKeyReport_16 = _mediaKeyReport[1] | (_mediaKeyReport[0] << 8); mediaKeyReport_16 &= ~k_16; _mediaKeyReport[0] = (uint8_t)((mediaKeyReport_16 & 0xFF00) >> 8); _mediaKeyReport[1] = (uint8_t)(mediaKeyReport_16 & 0x00FF); sendReport(&_mediaKeyReport); return 1;
}

// YENİ: Güç tuşuna basma ve bırakma fonksiyonları
void BleComboKeyboard::consumerPress(uint8_t k) {
  _consumerKeyReport = k;
  sendReport(&_consumerKeyReport);
}

void BleComboKeyboard::consumerRelease(void) {
  _consumerKeyReport = 0x00;
  sendReport(&_consumerKeyReport);
}


void BleComboKeyboard::releaseAll(void) {
	_keyReport.keys[0] = 0; _keyReport.keys[1] = 0; _keyReport.keys[2] = 0; _keyReport.keys[3] = 0; _keyReport.keys[4] = 0; _keyReport.keys[5] = 0;
	_keyReport.modifiers = 0;
    _mediaKeyReport[0] = 0; _mediaKeyReport[1] = 0;
    _consumerKeyReport = 0; // YENİ
	sendReport(&_keyReport);
    sendReport(&_mediaKeyReport);
    sendReport(&_consumerKeyReport); // YENİ
}

size_t BleComboKeyboard::write(uint8_t c) {
	uint8_t p = press(c); release(c); return p;
}

size_t BleComboKeyboard::write(const MediaKeyReport c) {
	uint16_t p = press(c); release(c); return p;
}

size_t BleComboKeyboard::write(const uint8_t *buffer, size_t size) {
	size_t n = 0; while (size--) { if (*buffer != '\r') { if (write(*buffer)) { n++; } else { break; } } buffer++; } return n;
}