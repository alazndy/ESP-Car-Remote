#include <Arduino.h>
// Sizin lib klasörünüzdeki kütüphanenin doğru başlık dosyaları bunlardır.
// Bu sefer derleyici bunları kesinlikle bulacaktır.
#include <BleComboKeyboard.h>
#include <BleComboMouse.h>

// ===================================================================================
// --- KONTROL PANELİ ---
// Projeyi SİMÜLATÖR ile test etmek için bu ayarı 'true' yapın.
// Fiziksel butonları ve joystick'i bağladığınızda ise 'false' yapın.
#define SIMULATOR_MODE true
// ===================================================================================


#if !SIMULATOR_MODE
// --- FİZİKSEL MOD İÇİN Pin Tanımlamaları ---
#define JOY_X_PIN   34
#define JOY_Y_PIN   35
#define JOY_SW_PIN  32
#define BACK_PIN         25
#define PLAY_PAUSE_PIN   26
#define ASSISTANT_PIN    27
#define VOL_UP_PIN       14
#define VOL_DOWN_PIN     12
#define NEXT_TRACK_PIN   13
#define PREV_TRACK_PIN   15
#endif


// --- Doğru Sınıf İsimleriyle ve Doğru Yapıda Cihaz Tanımları ---
BleComboKeyboard bleKeyboard("ESP32 Medya & Sistem", "PlatformIO", 100);
BleComboMouse bleMouse(&bleKeyboard); // Kütüphanenin gerektirdiği doğru yapı

// --- Global Değişkenler ---
unsigned long lastStatusTime = 0;
unsigned long lastButtonPressTime = 0;
const int debounceDelay = 200;

// --- Ana Eylem Fonksiyonu ---
void handleCommand(char command) {
  Serial.print("[EYLEM] Komut işleniyor: '"); Serial.print(command); Serial.println("'");

  switch (command) {
    // Gezinti Komutları (Klavye Ok Tuşları olarak)
    case 'w': bleKeyboard.write(KEY_UP_ARROW); break;
    case 's': bleKeyboard.write(KEY_DOWN_ARROW);  break;
    case 'a': bleKeyboard.write(KEY_LEFT_ARROW); break;
    case 'd': bleKeyboard.write(KEY_RIGHT_ARROW);  break;
    case 't': bleKeyboard.write(KEY_RETURN);     break;

    // Medya ve Sistem Komutları
    case 'p': bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE); break;
    case 'y': bleKeyboard.write(KEY_MEDIA_VOLUME_UP); break;
    case 'l': bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN); break;
    case 'i': bleKeyboard.write(KEY_MEDIA_NEXT_TRACK); break;
    case 'g': bleKeyboard.write(KEY_MEDIA_PREVIOUS_TRACK); break;
    case 'b': bleKeyboard.write(KEY_MEDIA_WWW_BACK); break;
    
    // --- ASİSTAN İÇİN EN İYİ ALTERNATİFLER ---
    case 'x': 
      // Bu kütüphane Güç Tuşu'nu desteklemediği için, en iyi iki alternatif:
      // 1. Standart Asistan Tuşu (birçok cihazda çalışır)
      bleKeyboard.press(KEY_LEFT_GUI); 
      bleKeyboard.releaseAll();
      
      // 2. Samsung Rutinleri için Tetikleyici (daha garantili)
      // bleKeyboard.write(KEY_F12);
      break;
  }
}

#if !SIMULATOR_MODE
// --- Fiziksel Butonları ve Joystick'i Kontrol Eden Fonksiyon ---
void checkPhysicalInputs() {
  if (millis() - lastButtonPressTime < debounceDelay) { return; }
  if (digitalRead(JOY_SW_PIN) == LOW)      { handleCommand('t'); lastButtonPressTime = millis(); } 
  else if (digitalRead(BACK_PIN) == LOW)   { handleCommand('b'); lastButtonPressTime = millis(); } 
  else if (digitalRead(PLAY_PAUSE_PIN) == LOW) { handleCommand('p'); lastButtonPressTime = millis(); } 
  else if (digitalRead(ASSISTANT_PIN) == LOW)  { handleCommand('x'); lastButtonPressTime = millis(); } 
  else if (digitalRead(VOL_UP_PIN) == LOW)     { handleCommand('y'); lastButtonPressTime = millis(); } 
  else if (digitalRead(VOL_DOWN_PIN) == LOW)   { handleCommand('l'); lastButtonPressTime = millis(); } 
  else if (digitalRead(NEXT_TRACK_PIN) == LOW) { handleCommand('i'); lastButtonPressTime = millis(); } 
  else if (digitalRead(PREV_TRACK_PIN) == LOW) { handleCommand('g'); lastButtonPressTime = millis(); }
  int joyX = analogRead(JOY_X_PIN); int joyY = analogRead(JOY_Y_PIN);
  if (joyY < 1000)      { handleCommand('w'); lastButtonPressTime = millis(); } 
  else if (joyY > 3000) { handleCommand('s'); lastButtonPressTime = millis(); } 
  else if (joyX < 1000) { handleCommand('a'); lastButtonPressTime = millis(); } 
  else if (joyX > 3000) { handleCommand('d'); lastButtonPressTime = millis(); }
}
#endif


void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n--- KURULUM BAŞLADI ---");
  #if SIMULATOR_MODE
    Serial.println(">> MOD: Simülatör");
  #else
    Serial.println(">> MOD: Fiziksel Donanım");
    pinMode(JOY_SW_PIN, INPUT_PULLUP); pinMode(BACK_PIN, INPUT_PULLUP);
    pinMode(PLAY_PAUSE_PIN, INPUT_PULLUP); pinMode(ASSISTANT_PIN, INPUT_PULLUP);
    pinMode(VOL_UP_PIN, INPUT_PULLUP); pinMode(VOL_DOWN_PIN, INPUT_PULLUP);
    pinMode(NEXT_TRACK_PIN, INPUT_PULLUP); pinMode(PREV_TRACK_PIN, INPUT_PULLUP);
  #endif
  bleKeyboard.begin();
  Serial.println("--- KURULUM TAMAMLANDI ---");
  Serial.println("Cihaz reklam yapıyor. Bağlantı bekleniyor...");
}

void loop() {
  if (bleKeyboard.isConnected()) {
    if (millis() - lastStatusTime > 5000) {
      Serial.printf("[STATUS] Loop çalışıyor. Bağlantı: %s\n", bleKeyboard.isConnected() ? "EVET" : "HAYIR");
      lastStatusTime = millis();
    }
    #if SIMULATOR_MODE
      if (Serial.available() > 0) { handleCommand(Serial.read()); }
    #else
      checkPhysicalInputs();
    #endif
  } else {
    if (millis() - lastStatusTime > 5000) {
      Serial.println("[STATUS] Loop çalışıyor. Cihaz Bağlı: HAYIR...");
      lastStatusTime = millis();
    }
  }
}