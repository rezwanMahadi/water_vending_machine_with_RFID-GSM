#include <Arduino.h>
// #include <Wire.h>
// #include <LiquidCrystal_I2C.h>
// #include <SPI.h>
// #include <MFRC522.h>
// #include <Keypad.h>

// #define RST_PIN 17
// #define SS_PIN 16
// #define valve 15
// #define TRIGGER_PIN 5
// #define ECHO_PIN 35
// #define SENSOR 32

// const byte ROWS = 4;
// const byte COLS = 3;
// char keys[ROWS][COLS] = {
//   { '1', '2', '3' },
//   { '4', '5', '6' },
//   { '7', '8', '9' },
//   { '*', '0', '#' }
// };
// byte rowPins[ROWS] = { 13, 12, 14, 27 };
// byte colPins[COLS] = { 26, 25, 33 };
// Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// HardwareSerial serialSms(1);
// LiquidCrystal_I2C lcd(0x27, 20, 4);
// MFRC522 rfid(SS_PIN, RST_PIN);
// MFRC522::MIFARE_Key key2;

// struct UserData {
//   String uid;
//   String name;
//   float Tk;
// };

// UserData data[] = {
//   { "23c6d15", "Mr Sunny", 10.00 },
//   { "43452716", "Mr Kawsar", 10.00 },
// };

// // String data[][3] = {
// //   { "23c6d15", "Mr Sunny", "10" },
// //   { "43452716", "Mr Kawsar", "10" },
// // };
// volatile byte pulseCount;
// float perLiterTk = 1.0;

// void IRAM_ATTR pulseCounter() {
//   pulseCount++;
// }

// void printId(int index) {
//   lcd.clear();
//   lcd.setCursor(0, 0);
//   String u = "UID : " + data[index].uid;
//   lcd.print(u);
//   lcd.setCursor(0, 1);
//   String n = "Name : " + data[index].name;
//   lcd.print(n);
//   lcd.setCursor(0, 2);
//   String bal = "Balance : " + String(data[index].Tk) + " BDT";
//   lcd.print(bal);
//   delay(3000);
//   lcd.clear();
//   lcd.setCursor(0, 0);
//   lcd.print("ENTER OTP PRESS : *");
//   lcd.setCursor(0, 1);
//   lcd.print("FOR CONSUME WATER");
//   lcd.setCursor(0, 2);
//   lcd.print("TYPE THE AMOUNT or");
//   lcd.setCursor(0, 3);
//   lcd.print("PLACE HAND ON SONAR");
// }

// int sonar() {
//   digitalWrite(TRIGGER_PIN, LOW);
//   delayMicroseconds(2);
//   digitalWrite(TRIGGER_PIN, HIGH);
//   delayMicroseconds(10);
//   digitalWrite(TRIGGER_PIN, LOW);
//   long duration = pulseIn(ECHO_PIN, HIGH);
//   int distance = duration * 0.0343 / 2;
//   return distance;
// }

// float sonar_water(int index) {
//   long currentMillis = 0;
//   long previousMillis = 0;
//   int interval = 1000;
//   boolean ledState = LOW;
//   float calibrationFactor = 4.5;
//   byte pulse1Sec = 0;
//   float flowRate;
//   unsigned int flowMilliLitres = 0;
//   unsigned int totalMilliLitres = 0;
//   float liter = 0.0;
//   float tk = 0.0;
//   lcd.clear();
//   lcd.setCursor(0, 0);
//   lcd.print("PROCESSINNG.........");
//   digitalWrite(valve, HIGH);
//   delay(2000);
//   while (true) {
//     int distance = sonar();
//     currentMillis = millis();
//     if (currentMillis - previousMillis > interval) {
//       lcd.clear();
//       pulse1Sec = pulseCount;
//       pulseCount = 0;
//       flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
//       previousMillis = millis();
//       flowMilliLitres = (flowRate / 60) * 1000;
//       totalMilliLitres += flowMilliLitres;
//       liter = static_cast<float>(totalMilliLitres) / 1000.0;
//       tk = (perLiterTk/1000.0)*static_cast<float>(totalMilliLitres);
//       String p = "CONSUMPTION: " + String(liter, 3) + " L";
//       String t = "TOTAL TK : " + String(tk, 3) + " BDT";
//       lcd.setCursor(0, 0);
//       lcd.print(p);
//       lcd.setCursor(0, 1);
//       lcd.print(t);
//     }
//     if (distance <= 4 || tk>=data[index].Tk) {
//       data[index].Tk = data[index].Tk - tk;
//       digitalWrite(valve, LOW);
//       break;
//     }
//   }
//   delay(2000);
//   lcd.clear();
//   return tk;
// }

// // void keypad_water() {
// // }

// // void otp_confirm() {
// // }

// void setup() {
//   serialSms.begin(9600, SERIAL_8N1, 2, 4);
//   Serial.begin(115200);
//   SPI.begin();
//   rfid.PCD_Init();
//   lcd.init();
//   lcd.backlight();
//   lcd.clear();
//   pinMode(valve, OUTPUT);
//   pinMode(TRIGGER_PIN, OUTPUT);
//   pinMode(ECHO_PIN, INPUT);
//   pinMode(SENSOR, INPUT);
//   digitalWrite(valve, LOW);
//   delay(1000);
//   attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, RISING);
// }

// void loop() {
//   if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
//     String uid = "";
//     for (byte i = 0; i < rfid.uid.size; i++) {
//       uid += String(rfid.uid.uidByte[i], HEX);
//     }
//     int index = -1;
//     for (int i = 0; i < sizeof(data) / sizeof(data[0]); i++) {
//       if (uid == data[i].uid) {
//         index = i;
//         break;
//       }
//     }
//     if (index != -1) {
//       printId(index);
//       while (data[index].uid.toInt() > 0) {
//         int distance = sonar();
//         char key = keypad.getKey();
//         if (key) {
//           if (key == '*') {
//             digitalWrite(valve, HIGH);
//             delay(2000);
//             digitalWrite(valve, LOW);
//             lcd.clear();
//             break;
//           }
//         } else if (distance <= 4) {
//           float tk = sonar_water(index);
//           break;
//         }
//       }
//     } else {
//       lcd.clear();
//       lcd.setCursor(0, 0);
//       lcd.print("UID not found");
//       delay(2000);
//       lcd.clear();
//     }
//     rfid.PICC_HaltA();
//     rfid.PCD_StopCrypto1();
//   } else {
//     lcd.setCursor(4, 1);
//     lcd.print("WATER VENDING");
//     lcd.setCursor(6, 2);
//     lcd.print("MACHINE");
//     lcd.setCursor(0, 3);
//   }
// }

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>

#define RST_PIN 17
#define SS_PIN 16
#define valve 15
#define TRIGGER_PIN 5
#define ECHO_PIN 35
#define SENSOR 32

const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  { '1', '2', '3' },
  { '4', '5', '6' },
  { '7', '8', '9' },
  { '*', '0', '#' }
};
byte rowPins[ROWS] = { 13, 12, 14, 27 };
byte colPins[COLS] = { 26, 25, 33 };
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

HardwareSerial serialSms(1);
LiquidCrystal_I2C lcd(0x27, 20, 4);
MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key2;

struct UserData {
  String uid;
  String name;
  int tk;
};

UserData data[] = {
  { "23c6d15", "Mr Sunny", 10 },
  { "43452716", "Mr Kawsar", 10 }
};

volatile byte pulseCount;
float perLiterTk = 1.0;

void IRAM_ATTR pulseCounter() {
  pulseCount++;
}

void printUserData(int index) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("UID : " + data[index].uid);
  lcd.setCursor(0, 1);
  lcd.print("Name: " + data[index].name);
  lcd.setCursor(0, 2);
  lcd.print("Balance: " + String(data[index].tk) + " BDT");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ENTER OTP PRESS : 1");
  lcd.setCursor(0, 1);
  lcd.print("TYPE AMOUNT PRESS :2");
  lcd.setCursor(0, 2);
  lcd.print("PLACE HAND ON SONAR");
}

int sonar() {
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  int distance = duration * 0.0343 / 2;
  return distance;
}

float sonar_water(int index) {
  long currentMillis = 0;
  long previousMillis = 0;
  int interval = 1000;
  boolean ledState = LOW;
  float calibrationFactor = 4.5;
  byte pulse1Sec = 0;
  float flowRate;
  unsigned int flowMilliLitres = 0;
  unsigned int totalMilliLitres = 0;
  float liter = 0.0;
  float tk = 0.0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PROCESSINNG.........");
  digitalWrite(valve, HIGH);
  delay(2000);
  while (true) {
    int distance = sonar();
    currentMillis = millis();
    if (currentMillis - previousMillis > interval) {
      lcd.clear();
      pulse1Sec = pulseCount;
      pulseCount = 0;
      flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
      previousMillis = millis();
      flowMilliLitres = (flowRate / 60) * 1000;
      totalMilliLitres += flowMilliLitres;
      liter = static_cast<float>(totalMilliLitres) / 1000.0;
      tk = (perLiterTk / 1000.0) * static_cast<float>(totalMilliLitres);
      String p = "CONSUMPTION: " + String(liter, 3) + " L";
      String t = "TOTAL TK : " + String(tk, 3) + " BDT";
      lcd.setCursor(0, 0);
      lcd.print(p);
      lcd.setCursor(0, 1);
      lcd.print(t);
    }
    if (distance <= 4 || tk >= data[index].tk) {
      data[index].tk = data[index].tk - tk;
      digitalWrite(valve, LOW);
      break;
    }
  }
  delay(2000);
  lcd.clear();
  return tk;
}

void setup() {
  serialSms.begin(9600, SERIAL_8N1, 2, 4);
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  pinMode(valve, OUTPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(SENSOR, INPUT);
  digitalWrite(valve, LOW);
  delay(1000);
  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, RISING);
}

void loop() {
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String uid = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      uid += String(rfid.uid.uidByte[i], HEX);
    }
    int index = -1;
    for (int i = 0; i < sizeof(data) / sizeof(data[0]); i++) {
      if (uid == data[i].uid) {
        index = i;
        break;
      }
    }
    if (index != -1) {
      printUserData(index);
      while (data[index].tk > 0) {
        int distance = sonar();
        char key = keypad.getKey();
        if (key) {
          if (key == '1') {
            digitalWrite(valve, HIGH);
            delay(2000);
            digitalWrite(valve, LOW);
            lcd.clear();
            break;
          }
          else if(key=='2'){
          }
        } else if (distance <= 4) {
          float tk = sonar_water(index);
          break;
        }
      }
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("UID not found");
      delay(2000);
      lcd.clear();
    }
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  } else {
    lcd.setCursor(4, 1);
    lcd.print("WATER VENDING");
    lcd.setCursor(6, 2);
    lcd.print("MACHINE");
    lcd.setCursor(0, 3);
  }
}
