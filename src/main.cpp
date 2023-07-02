#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <HTTPClient.h>

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
    {"23c6d15", "Mr_Sani", 10, "01625006364", "", ""},
    {"43452716", "Mr_Kawsar", 10, "01604288065", "", ""}};

const char *ssid = "RedmiNote7";
const char *pass = "245025asdfjkl";
const char *server = "api.thingspeak.com";
String apiKey = "FVL26U4EBJGE2KNR";
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
bool isFractional = false; // Flag to track fractional mode
int fraction = 0;          // Variable to store the fractional part
String fractionValue = "";

void wificonnect()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CONNECTING WIFI");
  lcd.setCursor(0, 1);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    lcd.setCursor(i, 1);
    lcd.print(".");
    i++;
  }
  i = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WIFI CONNECTED");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(3000);
  lcd.clear();
}

void send_thingspeak(int index, int case_number)
{
  String url = "";
  switch (case_number)
  {
  case 1:
    url = "http://" + String(server) + "/update?api_key=" + String(apiKey) + "&field1=" +
          data[index].uid + "&field2=" + data[index].name + "&field4=" + String(data[index].tk) +
          "&field5=" + data[index].tk_buffer;
    break;

  default:
    url = "http://" + String(server) + "/update?api_key=" + String(apiKey) + "&field1=" +
          data[index].uid + "&field2=" + data[index].name + "&field3=" + String(liter) +
          "&field4=" + String(data[index].tk);
    break;
  }
  Serial.println();
  Serial.println(url);
  HTTPClient http;
  http.begin(url);
  int httpResponseCode = http.GET();
  if (httpResponseCode == 200)
  {
    Serial.println("Data sent successfully!");
  }
  else
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

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
  String c = "COST : " + String(l * perLiterTk, 3) + "Tk";
  lcd.print(c);
  lcd.setCursor(0, 3);
  String ntk = "BALANCE : " + String(data[index].tk) + "Tk";
  lcd.print(ntk);
  send_thingspeak(index, 0);
  currentMillis = 0;
  previousMillis = 0;
  pulse1Sec = 0;
  flowRate = 0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  liter = 0.0;
  tk = 0.0;
  delay(5000);
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
      if ((key >= '0' && key <= '9') || key == '*')
      {
        if (key == '*')
        {
          // Toggle fractional mode
          i++;
          isFractional = true;
          lcd.setCursor(i, 2);
          lcd.print('.'); // Print decimal point on LCD
        }
        else
        {
          if (!isFractional)
          {
            value = value * 10 + (key - '0');
            i++;
            lcd.setCursor(i, 2);
            lcd.print(key);
          }
          else
          {
            fraction = fraction * 10 + (key - '0');
            i++;
            lcd.setCursor(i, 2);
            lcd.print(key);
            fractionValue += key;
          }
        }
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
        String t = "0." + fractionValue;
        float totalValue = value + t.toFloat();
        float money = totalValue * perLiterTk;
        if (money <= data[index].tk)
        {
          while (true)
          {
            digitalWrite(valve, HIGH);
            if (measure_water() >= totalValue)
            {
              print_consumed_water(index, liter);
              delay(2000);
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
    for (int index_finder = 0; index_finder < sizeof(data) / sizeof(data[0]); index_finder++)
    {
      if (sender == data[index_finder].number)
      {
        index = index_finder;
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
  serialsms.println("AT"); // Once the handshake test is successful, it will return "OK"
  updateSerial();
  delay(1000);
  serialsms.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  delay(1000);
  serialsms.println("AT+CNMI=1,2,0,0,0"); // Decides how newly arrived SMS messages should be handled
  updateSerial();
  delay(1000);
  lcd.setCursor(5, 0);
  lcd.print("PROJECT OF");
  lcd.setCursor(0, 1);
  lcd.print("SANI  ID: ET183045");
  lcd.setCursor(0, 2);
  lcd.print("KAWSAR  ID: ET183041");
  delay(5000);
  lcd.clear();
  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, RISING);
}

void loop()
{
  while (WiFi.status() == WL_CONNECTED)
  {
    if (serialsms.available())
    {
      updateSerial();
    }
    else if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial())
    {
      String uid = "";
      for (byte i = 0; i < rfid.uid.size; i++)
      {
        uid += String(rfid.uid.uidByte[i], HEX);
      }
      int index = -1;
      for (int index_finder = 0; index_finder < sizeof(data) / sizeof(data[0]); index_finder++)
      {
        if (uid == data[index_finder].uid)
        {
          index = index_finder;
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
                    send_thingspeak(index, 1);
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
  wificonnect();
}
