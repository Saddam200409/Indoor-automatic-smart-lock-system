#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define SW420_PIN   3
#define BUZZER_PIN  5

// Buzzer Active LOW
#define ON   LOW
#define OFF  HIGH

LiquidCrystal_I2C lcd(0x27, 16, 2);
int getarCount = 0;

void setup() {
  pinMode(SW420_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, OFF);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("TEST GETARAN");
  delay(1000); lcd.clear();
}

void loop() {
  lcd.setCursor(0,0); lcd.print("Monitor Getar...");
  lcd.setCursor(0,1); lcd.print("Level: " + String(getarCount) + "/10");

  if (digitalRead(SW420_PIN) == HIGH) {
    getarCount++;
    
    // Tampilan Ada Getaran
    lcd.clear();
    lcd.print("ADA GETARAN!!");
    lcd.setCursor(0,1); 
    lcd.print("Level: " + String(getarCount) + "/10");
    
    // Bunyi Bip Pendek
    digitalWrite(BUZZER_PIN, ON);
    delay(200);
    digitalWrite(BUZZER_PIN, OFF);
    
    delay(200); // Debounce sedikit

    // JIKA SUDAH 10 KALI -> BLOKIR
    if (getarCount >= 10) {
      blokirSistem();
    }
  }
}

void blokirSistem() {
  lcd.clear();
  lcd.print("SYSTEM LOCKED!");
  lcd.setCursor(0,1); lcd.print("HARUS RESTART");
  
  // Loop selamanya (Macet)
  while (true) {
    digitalWrite(BUZZER_PIN, ON); // Alarm Nyala
    delay(500);
    digitalWrite(BUZZER_PIN, OFF); // Alarm Mati
    delay(500);
  }
}