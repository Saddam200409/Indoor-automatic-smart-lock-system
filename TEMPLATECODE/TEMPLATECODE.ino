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

// Ubah urutan angka di dalam kurung kurawal ini:
byte rowPins[ROWS] = {25, 24, 23, 22};
byte colPins[COLS] = {26, 27, 28, 29}; 
Keypad myKeypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

/* ================= VARIABEL ================= */
String inputPassword = "";
String PASSWORD_BENAR = "123456";

int fingerFail = 0;
int pinFail = 0;
int getarCount = 0; 

// VARIABEL BARU: STATUS VERIFIKASI PERTAMA
bool fingerPassed = false; 

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

  // LOGIKA 2 TAHAP
  if (fingerPassed == false) {
    // Tahap 1: Wajib Fingerprint Dulu
    cekFingerprint();
  } else {
    // Tahap 2: Baru Boleh Input Keypad
    cekKeypad();
  }
}

/* ================= FINGERPRINT (VERIFIKASI 1) ================= */
void cekFingerprint() {
  // Tampilan Standby Tahap 1
  lcd.setCursor(0,0);
  lcd.print("1. Scan Jari... ");
  lcd.setCursor(0,1);
  lcd.print("                "); // Kosongkan baris bawah

  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return;

  p = finger.fingerFastSearch();
  
  if (p == FINGERPRINT_OK) {
    // JIKA JARI BENAR -> LANJUT KE TAHAP 2
    lcd.clear();
    lcd.print("Finger OK!");
    lcd.setCursor(0,1);
    lcd.print("Lanjut ke PIN >>");
    
    // Bunyi Bip Pendek
    digitalWrite(BUZZER_PIN, LOW); delay(100); digitalWrite(BUZZER_PIN, HIGH);
    delay(1000);
    
    fingerPassed = true; // KUNCI UTAMA: Buka akses ke Keypad
    lcd.clear();
    
  } else {
    // JIKA JARI SALAH
    fingerFail++;
    lcd.clear();
    lcd.print("Jari Ditolak!");
    lcd.setCursor(0,1);
    lcd.print("Coba Lagi...");
    
    // Bunyi Gagal
    digitalWrite(BUZZER_PIN, LOW); delay(500); digitalWrite(BUZZER_PIN, HIGH);
    delay(1000); 
    lcd.clear();

    // Reset counter finger agar user bisa coba terus sampai benar
    if (fingerFail >= 3) {
       fingerFail = 0; 
    }
  }
}

/* ================= KEYPAD (VERIFIKASI 2) ================= */
void cekKeypad() {
  // Tampilan Standby Tahap 2
  lcd.setCursor(0,0);
  lcd.print("2. Masukkan PIN:");
  
  char key = myKeypad.getKey();
  if (!key) return;

  // Bunyi Tik Tiap Tombol (INI VERSI LAMA YANG MASIH BUNYI)
  digitalWrite(BUZZER_PIN, LOW); delay(50); digitalWrite(BUZZER_PIN, HIGH);

  if (key == '#') {
    if (inputPassword == PASSWORD_BENAR) {
      // JIKA PIN BENAR -> PINTU BARU TERBUKA
      lcd.clear();
      lcd.print("VERIFIKASI SUKSES");
      lcd.setCursor(0,1);
      lcd.print("Pintu Terbuka");
      
      // Reset Semua Status
      pinFail = 0;
      fingerFail = 0; 
      getarCount = 0;
      fingerPassed = false; // Reset balik ke tahap 1
      inputPassword = "";
      
      aksesBerhasil(); // Buka Solenoid
    } else {
      // JIKA PIN SALAH
      pinFail++;
      lcd.clear();
      lcd.print("PIN Salah!");
      delay(1000);
      
      if (pinFail >= 3) {
        blokirSementara(); // Blokir 10 detik
        fingerPassed = false; // Kembalikan ke tahap awal fingerprint
      } else {
        lcd.clear();
      }
    }
    inputPassword = "";
  }

  else if (key == '*') {
    // Tombol Batal -> Kembali ke Fingerprint
    inputPassword = "";
    fingerPassed = false; 
    lcd.clear();
    lcd.print("Dibatalkan.");
    delay(1000);
    lcd.clear();
  }

  else if (key >= '0' && key <= '9') {
    if (inputPassword.length() < 6) {
      inputPassword += key;
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

/* ================= SENSOR GETAR ================= */
void cekGetar() {
  // Jika sensor getar mendeteksi gerakan (HIGH)
  if (digitalRead(SW420_PIN) == HIGH) {
    getarCount++; // Tambah hitungan

    // Tampilkan Peringatan di LCD
    lcd.clear();
    lcd.print("ADA PAKSAAN!!");
    lcd.setCursor(0, 1);
    
    // Tampilan Level per 10
    lcd.print("Level: " + String(getarCount) + "/10");

    // Bunyi Buzzer 1x (Pendek)
    digitalWrite(BUZZER_PIN, LOW);   // Nyala
    delay(200);
    digitalWrite(BUZZER_PIN, HIGH);  // Mati
    
    // Jeda sebentar biar tidak menghitung 1 getaran jadi banyak
    delay(500); 

    // Jika sudah 5 kali getaran (Sesuai kode awalmu)
    if (getarCount >= 5) {
      blokirPermanen(); // Masuk ke fungsi blokir total
    }
    
    // Kembalikan tampilan LCD sesuai Status
    lcd.clear();
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
  fingerPassed = false; // Reset status verifikasi
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