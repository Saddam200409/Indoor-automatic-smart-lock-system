#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>

#define RELAY_PIN   4
#define BUZZER_PIN  5

// RELAY & BUZZER ACTIVE LOW
#define ON   LOW
#define OFF  HIGH

LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial1); // RX=19, TX=18

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, OFF);
  digitalWrite(BUZZER_PIN, OFF);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("TEST FINGERPRINT");

  Serial.begin(9600);
  finger.begin(57600);
  
  if (finger.verifyPassword()) {
    lcd.setCursor(0,1); lcd.print("Sensor OK!");
  } else {
    lcd.setCursor(0,1); lcd.print("Sensor ERROR!");
    while(1);
  }
  delay(1000); lcd.clear();
}

void loop() {
  lcd.setCursor(0,0); lcd.print("Tempel Jari...");
  
  int result = getFingerprintID();
  
  if (result != -1) {
    // JIKA JARI DITEMUKAN (HASIL POSITIF)
    lcd.clear();
    lcd.print("Jari Diterima!");
    lcd.setCursor(0,1);
    lcd.print("ID: " + String(result));
    
    // Buka Solenoid
    digitalWrite(RELAY_PIN, ON);
    delay(3000); // Buka 3 detik
    digitalWrite(RELAY_PIN, OFF);
    lcd.clear();
  }
}

int getFingerprintID() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1; // Tidak ada jari

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) {
    // JIKA JARI ADA TAPI TIDAK COCOK
    lcd.clear();
    lcd.print("Jari Ditolak!");
    
    // Bunyi Buzzer Gagal
    digitalWrite(BUZZER_PIN, ON);
    delay(1000);
    digitalWrite(BUZZER_PIN, OFF);
    lcd.clear();
    return -1;
  }
  
  // Jika cocok, kembalikan ID
  return finger.fingerID;
}