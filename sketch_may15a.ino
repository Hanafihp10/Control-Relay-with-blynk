#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <LiquidCrystal_I2C.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define RELAY_PIN 25
#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS 2
#define LED_PIN 2
#define BUZZER_PIN 33
#define VIRTUAL_LCD_LINE1 V2
#define VIRTUAL_LCD_LINE2 V3

char auth[] = "eAnX45YwGK8Z3wJApeMF5ZRmnSL2jvEh";
char ssid[] = "GALAXY KOST A";
char pass[] = "@CyberOne001";

LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);
BlynkTimer timer;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 25200, 60000); // UTC+7 (WIB)

int countdownTime = 0;
bool timerActive = false;
bool manualOverride = false;

// Input dari Terminal di V0
BLYNK_WRITE(V0) {
  String input = param.asStr();
  countdownTime = input.toInt();

  if (countdownTime > 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Set: ");
    lcd.print(countdownTime);
    lcd.print("s");

    Blynk.virtualWrite(VIRTUAL_LCD_LINE1, "Set: " + String(countdownTime) + "s");
    Blynk.virtualWrite(VIRTUAL_LCD_LINE2, "");

    Blynk.virtualWrite(V0, "Waktu diset: " + String(countdownTime) + " detik");
    delay(500);
    Blynk.virtualWrite(V0, ""); // Bersihkan terminal setelah tampil sebentar

  } else {
    Blynk.virtualWrite(V0, "Input tidak valid!");
  }
}

// Tombol start dari Blynk V1 (Timer mode)
BLYNK_WRITE(V1) {
  int state = param.asInt();
  if (state == 1 && countdownTime > 0) {
    timerActive = true;
    manualOverride = false;  // Matikan override jika sebelumnya ON
    digitalWrite(RELAY_PIN, HIGH); // Relay ON (NC - aktif LOW)
    tone(BUZZER_PIN, 2000, 300); // Buzzer bunyi awal
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Mulai Timer");

    Blynk.virtualWrite(VIRTUAL_LCD_LINE1, "Mulai Timer");
    Blynk.virtualWrite(VIRTUAL_LCD_LINE2, "");
  }
}

// Tombol manual override di V4 (tanpa timer)
BLYNK_WRITE(V4) {
  int state = param.asInt();
  if (state == 1) {
    manualOverride = true;
    timerActive = false;
    digitalWrite(RELAY_PIN, HIGH); // Relay ON (aktif LOW)
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("KONDISI : ON");

    Blynk.virtualWrite(VIRTUAL_LCD_LINE1, "KONDISI : ON");
    Blynk.virtualWrite(VIRTUAL_LCD_LINE2, "");
  } else {
    manualOverride = false;
    digitalWrite(RELAY_PIN, LOW); // Relay OFF (NC - tidak aktif HIGH)
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("KONDISI : OFF");

    Blynk.virtualWrite(VIRTUAL_LCD_LINE1, "KONDISI : OFF");
    Blynk.virtualWrite(VIRTUAL_LCD_LINE2, "");
  }
}

void countDownFunction() {
  timeClient.update();
  String currentTime = timeClient.getFormattedTime();

  lcd.setCursor(0, 1);
  lcd.print(currentTime);

  if (!manualOverride && timerActive && countdownTime > 0) {
    countdownTime--;
    lcd.setCursor(0, 0);
    lcd.print("Sisa: ");
    lcd.print(countdownTime);
    lcd.print("s   ");

    Blynk.virtualWrite(VIRTUAL_LCD_LINE1, "Sisa: " + String(countdownTime) + "s");
    Blynk.virtualWrite(VIRTUAL_LCD_LINE2, "Jam: " + currentTime);

    if (countdownTime == 0) {
      digitalWrite(RELAY_PIN, LOW); // Relay OFF (NC - tidak aktif HIGH)
      timerActive = false;
      tone(BUZZER_PIN, 1000, 500); // Buzzer selesai
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Timer Selesai");
      lcd.setCursor(0, 1);
      lcd.print("Stopkontak OFF");

      Blynk.virtualWrite(VIRTUAL_LCD_LINE1, "Timer Selesai");
      Blynk.virtualWrite(VIRTUAL_LCD_LINE2, "Stopkontak : OFF");
    }
  } else {
    Blynk.virtualWrite(VIRTUAL_LCD_LINE2, "Jam: " + currentTime);
  }
}

void setup() { 
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Relay OFF default (LOW = OFF untuk HIGH trigger relay)
  digitalWrite(LED_PIN, HIGH);

  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Menghubungkan...");

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Terhubung");
  Blynk.begin(auth, ssid, pass, "blynk.my.id", 8182);
  timeClient.begin();
  timer.setInterval(1000L, countDownFunction);
}

void loop() {
  Blynk.run();
  timer.run();
}