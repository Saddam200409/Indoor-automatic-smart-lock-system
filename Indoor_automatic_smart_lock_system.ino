#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Adafruit_Fingerprint.h>

/* ================= PIN ================= */
#define SW420_PIN   3
#define RELAY_PIN   4
#define BUZZER_PIN  5

/* ===== RELAY ACTIVE LOW (WAJIB) ===== */
#define RELAY_ON   LOW
#define RELAY_OFF  HIGH

/* ================= LCD ================= */
LiquidCrystal_I2C lcd(0x27, 16, 2);

/* ================= FINGERPRINT ================= */
// Arduino Mega RX1=19 TX1=18
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial1);

/* ================= KEYPAD ================= */
const byte ROWS = 4; 
const byte COLS = 4; 
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {22, 23, 24, 25}; 
byte colPins[COLS] = {26, 27, 28, 29}; 
Keypad myKeypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

/* ================= VARIABEL ================= */
String inputPassword = "";
String PASSWORD_BENAR = "123456";

int fingerFail = 0;
int pinFail = 0;
int getarCount = 0; // Variabel baru untuk menghitung getaran

/* ================= SETUP ================= */
void setup() {
  pinMode(SW420_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, RELAY_OFF);

  // Buzzer ACTIVE-LOW → harus HIGH saat idle
  digitalWrite(BUZZER_PIN, HIGH);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Sistem Siap");

  Serial.begin(9600);
  Serial1.begin(57600);
  finger.begin(57600);

  delay(2000);
  lcd.clear();
}

/* ================= LOOP ================= */
void loop() {
  cekGetar(); // Cek getaran terus menerus

  if (fingerFail < 3) {
    cekFingerprint();
  } else {
    cekKeypad();
  }
}

/* ================= FINGERPRINT ================= */
void cekFingerprint() {
  lcd.setCursor(0,0);
  lcd.print("Tempelkan Jari ");

  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return;

  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    fingerFail = 0;
    getarCount = 0; // Reset hitungan getar jika sukses masuk
    aksesBerhasil();
  } else {
    fingerFail++;
    lcd.clear();
    lcd.print("Finger Gagal!");
    delay(1000); 
    
    if (fingerFail >= 3) {
      lcd.clear();
      lcd.print("Finger Limit!");
      lcd.setCursor(0,1);
      lcd.print("Gunakan PIN...");
      delay(2000);
      lcd.clear();
      lcd.print("Masukkan PIN:");
      lcd.setCursor(0,1);
    }
  }
}

/* ================= KEYPAD ================= */
void cekKeypad() {
  char key = myKeypad.getKey();
  if (!key) return;

  if (key == '#') {
    if (inputPassword == PASSWORD_BENAR) {
      lcd.clear();
      lcd.print("PIN Benar");
      lcd.setCursor(0,1);
      lcd.print("Pintu Terbuka");
      
      // Reset Semua Counter jika berhasil
      pinFail = 0;
      fingerFail = 0; 
      getarCount = 0;
      
      aksesBerhasil();
    } else {
      pinFail++;
      lcd.clear();
      lcd.print("PIN Salah!");
      delay(1000);
      
      if (pinFail >= 3) {
        blokirSementara(); // Blokir 10 detik (Salah PIN)
      } else {
        lcd.clear();
        lcd.print("Masukkan PIN:");
        lcd.setCursor(0,1);
      }
    }
    inputPassword = "";
  }

  else if (key == '*') {
    inputPassword = "";
    lcd.setCursor(0,1);
    lcd.print("                ");
    lcd.setCursor(0,1);
  }

  else if (key >= '0' && key <= '9') {
    if (inputPassword.length() < 6) {
      inputPassword += key;
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      for (int i = 0; i < inputPassword.length(); i++) {
        lcd.print('*');
      }
    }
  }
}

/* ================= SOLENOID ================= */
void aksesBerhasil() {
  digitalWrite(RELAY_PIN, RELAY_ON);
  delay(5000);
  digitalWrite(RELAY_PIN, RELAY_OFF);
  lcd.clear();
}

/* ================= SENSOR GETAR (LOGIKA BARU) ================= */
void cekGetar() {
  // Jika sensor getar mendeteksi gerakan (HIGH)
  if (digitalRead(SW420_PIN) == HIGH) {
    getarCount++; // Tambah hitungan

    // Tampilkan Peringatan di LCD
    lcd.clear();
    lcd.print("ADA PAKSAAN!!");
    lcd.setCursor(0, 1);
    lcd.print("Level: " + String(getarCount) + "/5");

    // Bunyi Buzzer 1x (Pendek)
    digitalWrite(BUZZER_PIN, LOW);   // Nyala
    delay(200);
    digitalWrite(BUZZER_PIN, HIGH);  // Mati
    
    // Jeda sebentar biar tidak menghitung 1 getaran jadi banyak
    delay(500); 

    // Jika sudah 5 kali getaran
    if (getarCount >= 5) {
      blokirPermanen(); // Masuk ke fungsi blokir total
    }
    
    // Kembalikan tampilan LCD
    lcd.clear();
    if(fingerFail >= 3) {
       lcd.print("Masukkan PIN:");
    } else {
       lcd.print("Tempelkan Jari");
    }
  }
}

/* ================= BLOKIR SEMENTARA (SALAH PIN) ================= */
void blokirSementara() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SYSTEM BLOCKED!!");
  lcd.setCursor(0,1);
  lcd.print("Tunggu 10 Detik");

  digitalWrite(BUZZER_PIN, LOW); 
  delay(10000); 
  digitalWrite(BUZZER_PIN, HIGH); 

  fingerFail = 0;
  pinFail = 0;
  inputPassword = "";
  
  lcd.clear();
  lcd.print("Silakan Coba");
  lcd.setCursor(0,1);
  lcd.print("Lagi...");
  delay(2000);
  lcd.clear();
}

/* ================= BLOKIR PERMANEN (SENSOR GETAR) ================= */
void blokirPermanen() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SYSTEM LOCKED!");
  lcd.setCursor(0,1);
  lcd.print("HARUS RESTART");

  // Infinite Loop (Program macet di sini selamanya sampai dimatikan)
  while (true) {
    // Alarm bunyi putus-putus tanda bahaya
    digitalWrite(BUZZER_PIN, LOW);  // Nyala
    delay(500);
    digitalWrite(BUZZER_PIN, HIGH); // Mati
    delay(500);
  }
}