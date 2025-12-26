#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

#define RELAY_PIN   4
#define BUZZER_PIN  5

#define ON   LOW
#define OFF  HIGH

LiquidCrystal_I2C lcd(0x27, 16, 2);

const byte ROWS = 4; 
const byte COLS = 4; 
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// Urutan Pin Lurus 22-29
byte rowPins[ROWS] = {22, 23, 24, 25}; 
byte colPins[COLS] = {26, 27, 28, 29}; 
Keypad myKeypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String inputPassword = "";
String PASSWORD_BENAR = "123456";

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, OFF);
  digitalWrite(BUZZER_PIN, OFF);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("TEST KEYPAD");
  delay(1000); lcd.clear();
}

void loop() {
  lcd.setCursor(0,0); lcd.print("Input PIN:");
  
  char key = myKeypad.getKey();
  
  if (key) {
    if (key == '#') {
      if (inputPassword == PASSWORD_BENAR) {
        // PIN BENAR
        lcd.clear();
        lcd.print("PIN BENAR!");
        lcd.setCursor(0,1); lcd.print("Pintu Terbuka");
        
        digitalWrite(RELAY_PIN, ON);
        delay(3000);
        digitalWrite(RELAY_PIN, OFF);
      } else {
        // PIN SALAH
        lcd.clear();
        lcd.print("PIN SALAH!");
        
        // Bunyi Buzzer
        digitalWrite(BUZZER_PIN, ON);
        delay(1000);
        digitalWrite(BUZZER_PIN, OFF);
      }
      inputPassword = ""; // Reset
      lcd.clear();
    } 
    else if (key == '*') {
      inputPassword = "";
      lcd.clear();
      lcd.print("Reset Input");
      delay(500); lcd.clear();
    }
    else {
      // Input Angka
      inputPassword += key;
      lcd.setCursor(0,1);
      for(int i=0; i<inputPassword.length(); i++) lcd.print("*");
    }
  }
}