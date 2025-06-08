#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>
#include <hd44780.h>               
#include <hd44780ioClass/hd44780_I2Cexp.h> 

// LCD setup
hd44780_I2Cexp lcd; // Automatically uses Wire with default I2C pins (21=SDA, 22=SCL)

// RFID setup (SPI for ESP32)
#define SS_PIN    5
#define RST_PIN   4
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Pins
#define BUZZER_PIN 27
#define BUTTON_PIN 26

int total = 0;

// Dummy database of items
struct Item {
  byte uid[4];
  int price;
  String name;
};

Item items[] = {
  {{0xDE, 0xAD, 0xBE, 0xEF}, 100, "Milk"},
  {{0xCA, 0xFE, 0xBA, 0xBE}, 50, "Bread"},
  {{0xFE, 0xED, 0xFA, 0xCE}, 75, "Eggs"}
};

const int numItems = sizeof(items) / sizeof(items[0]);

void setup() {
  Serial.begin(115200);
  SPI.begin(); // Init SPI bus
  mfrc522.PCD_Init(); // Init RFID

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  lcd.begin(16, 2);
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Smart Cart Ready");
  lcd.setCursor(0, 1);
  lcd.print("Scan to Start...");
}

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    checkButton();
    return;
  }

  bool found = false;

  for (int i = 0; i < numItems; i++) {
    if (memcmp(mfrc522.uid.uidByte, items[i].uid, 4) == 0) {
      total += items[i].price;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Added: " + items[i].name);
      lcd.setCursor(0, 1);
      lcd.print("Total: Rs ");
      lcd.print(total);
      tone(BUZZER_PIN, 1000, 200);
      delay(1500);
      found = true;
      break;
    }
  }

  if (!found) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Unknown Item!");
    tone(BUZZER_PIN, 300, 500);
    delay(1500);
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

void checkButton() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Checkout Done!");
    lcd.setCursor(0, 1);
    lcd.print("Total: Rs ");
    lcd.print(total);
    delay(4000);

    // Reset
    total = 0;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Smart Cart Ready");
    lcd.setCursor(0, 1);
    lcd.print("Scan to Start...");
  }
}
