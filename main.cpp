#include <Arduino.h>
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
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};
byte rowPins[ROWS] = {13, 12, 14, 27};
byte colPins[COLS] = {26, 25, 33};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

HardwareSerial serialsms(1);
LiquidCrystal_I2C lcd(0x27, 20, 4);
MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key2;

struct UserData
{
  String uid;
  String name;
  float tk;
  String number;
  String otp_buffer;
  String tk_buffer;
};

UserData data[] = {
    {"23c6d15", "Mr Sunny", 10, "01644644810", "", ""},
    {"43452716", "Mr Kawsar", 10, "01791154170", "", ""}};

long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
float calibrationFactor = 4.5;
byte pulse1Sec = 0;
float flowRate;
unsigned int flowMilliLitres = 0;
unsigned int totalMilliLitres = 0;
volatile byte pulseCount;
float liter = 0.0;
float tk = 0.0;
float perLiterTk = 1.0;
int i = 0;
String receivedMessage = "";

void IRAM_ATTR pulseCounter()
{
  pulseCount++;
}

void printUserData(int index)
{
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

int sonar()
{
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  int distance = duration * 0.0343 / 2;
  return distance;
}

void print_consumed_water(int index, float l)
{
  digitalWrite(valve, LOW);
  data[index].tk = data[index].tk - tk;
  lcd.clear();
  lcd.setCursor(0, 0);
  String n = "THANK YOU " + data[index].name;
  lcd.print(n);
  lcd.setCursor(0, 1);
  String lt = "CONSUMED : " + String(l, 3) + "L";
  lcd.print(lt);
  lcd.setCursor(0, 2);
  String ntk = "BALANCE : " + String(data[index].tk) + "Tk";
  lcd.print(ntk);
  currentMillis = 0;
  previousMillis = 0;
  pulse1Sec = 0;
  flowRate = 0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  liter = 0.0;
  tk = 0.0;
  delay(2000);
  lcd.clear();
}

float measure_water()
{
  currentMillis = millis();
  if (currentMillis - previousMillis > interval)
  {
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
  return liter;
}

void sonar_water(int index)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PROCESSINNG.........");
  digitalWrite(valve, HIGH);
  delay(2000);
  while (true)
  {
    measure_water();
    if (sonar() <= 4 || tk >= data[index].tk)
    {
      print_consumed_water(index, liter);
      break;
    }
  }
}

void key_water(int index)
{
  int value = 0;
  int exit = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TYPE THE AMOUNT AND");
  lcd.setCursor(0, 1);
  lcd.print("PRESS # :");
  lcd.setCursor(i, 2);
  lcd.blink();
  while (true)
  {
    char key = keypad.getKey();
    if (key)
    {
      if (key >= '0' && key <= '9')
      {
        value = value * 10 + (key - '0');
        i++;
        lcd.setCursor(i, 2);
        lcd.print(key);
      }
      else if (key == '#')
      {
        i = 0;
        lcd.noBlink();
        lcd.noCursor();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("PROCESSINNG.........");
        delay(2000);
        float money = value * perLiterTk;
        if (money <= data[index].tk)
        {
          while (true)
          {
            digitalWrite(valve, HIGH);
            if (measure_water() >= value)
            {
              print_consumed_water(index, liter);
              delay(2000);
              exit = 1;
              break;
            }
          }
        }
        else
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("NOT ENOUGH MONEY");
          delay(2000);
          lcd.clear();
          exit = 1;
        }
        break;
      }
    }
  }
}

void updateSerial();

void send_message(String number, String message)
{
  String recepiant = "AT+CMGS=\"+88" + number + "\"";
  serialsms.println(recepiant); // 1)
  updateSerial();
  delay(1000);
  serialsms.print(message); // 2) text content
  updateSerial();
  delay(1000);
  serialsms.write((char)26); // 3)
  updateSerial();
  delay(1000);
}

String generateOTP(int digits)
{
  String otp = "";
  for (int i = 0; i < digits; i++)
  {
    int randomDigit = random(0, 9);
    otp += String(randomDigit);
  }
  return otp;
}

void received_message(int delimiterIndex)
{
  if (delimiterIndex != -1)
  {
    String keyword1 = "You have received Tk";
    String keyword2 = "from";
    String tk = receivedMessage.substring((receivedMessage.indexOf(keyword1) + keyword1.length() + 1), receivedMessage.indexOf(keyword2) - 1);
    String sender = receivedMessage.substring(receivedMessage.indexOf(keyword2) + keyword2.length() + 1, receivedMessage.indexOf(keyword2) + keyword2.length() + 12);
    Serial.print("string: ");
    Serial.println(receivedMessage);
    int index = -1;
    for (int i = 0; i < sizeof(data) / sizeof(data[0]); i++)
    {
      if (sender == data[i].number)
      {
        index = i;
        break;
      }
    }
    if (sender == data[index].number && data[index].otp_buffer.isEmpty())
    {
      data[index].otp_buffer = generateOTP(6);
      data[index].tk_buffer = tk;
      String message = "Here is your OTP : " + data[index].otp_buffer;
      send_message(sender, message);
      receivedMessage = "";
      lcd.clear();
    }
  }
}

void updateSerial()
{
  while (Serial.available())
  {
    serialsms.write(Serial.read()); // Forward what Serial received to Software Serial Port
  }
  if (serialsms.available())
  {
    char receivedChar = serialsms.read(); // Read the received character
    if (receivedChar != '\r' && receivedChar != '\n')
    {
      receivedMessage += receivedChar; // Append the character to the received message
    }
    // Serial.write(serialsms.read());
    Serial.write(receivedChar);
    if (receivedChar == '\n')
    {
      int delimiterIndex = receivedMessage.indexOf(':');
      received_message(delimiterIndex);
      receivedMessage = "";
    }
  }
}

void setup()
{
  serialsms.begin(9600, SERIAL_8N1, 2, 4);
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
  serialsms.println("AT"); // Once the handshake test is successful, it will return "OK"
  updateSerial();
  delay(1000);
  serialsms.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  delay(1000);
  serialsms.println("AT+CNMI=1,2,0,0,0"); // Decides how newly arrived SMS messages should be handled
  updateSerial();
  delay(1000);
  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, RISING);
}

void loop()
{
  if (serialsms.available())
  {
    updateSerial();
  }
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial())
  {
    String uid = "";
    for (byte i = 0; i < rfid.uid.size; i++)
    {
      uid += String(rfid.uid.uidByte[i], HEX);
    }
    int index = -1;
    for (int i = 0; i < sizeof(data) / sizeof(data[0]); i++)
    {
      if (uid == data[i].uid)
      {
        index = i;
        break;
      }
    }
    if (index != -1)
    {
      printUserData(index);
      int value = 0;
      while (data[index].tk > 0)
      {
        char key = keypad.getKey();
        if (key)
        {
          if (key == '1')
          { // OTP
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("ENTER YOUR OTP AND");
            lcd.setCursor(0, 1);
            lcd.print("PRESS #");
            lcd.setCursor(i, 2);
            lcd.blink();
            while (true)
            {
              char key = keypad.getKey();
              if (key >= '0' && key <= '9')
              {
                value = value * 10 + (key - '0');
                i++;
                lcd.setCursor(i, 2);
                lcd.print(key);
              }
              else if (key == '#')
              {
                i = 0;
                lcd.noBlink();
                lcd.noCursor();
                if (value == data[index].otp_buffer.toInt())
                {
                  data[index].tk = data[index].tk + data[index].tk_buffer.toInt();
                  lcd.clear();
                  lcd.setCursor(0, 0);
                  String a = "THANK YOU " + data[index].name;
                  lcd.print(a);
                  lcd.setCursor(0, 1);
                  lcd.print("RECHARGE SUCCESSFUL");
                  lcd.setCursor(0, 2);
                  String b = "BALANCE:" + String(data[index].tk, 2) + "BDT";
                  lcd.print(b);
                  data[index].otp_buffer = "";
                  data[index].tk_buffer = "";
                  delay(3000);
                  lcd.clear();
                }
                else
                {
                  lcd.clear();
                  lcd.setCursor(0, 0);
                  lcd.print("OTP NOT MATCHED");
                  delay(3000);
                  lcd.clear();
                }
                break;
              }
            }
            break;
          }
          else if (key == '2')
          {
            key_water(index);
            break;
          }
        }
        if (sonar() <= 4)
        {
          sonar_water(index);
          break;
        }
      }
    }
    else
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("UID not found");
      delay(2000);
      lcd.clear();
    }
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }
  else
  {
    lcd.setCursor(4, 1);
    lcd.print("WATER VENDING");
    lcd.setCursor(6, 2);
    lcd.print("MACHINE");
    lcd.setCursor(0, 3);
  }
}
