//komen salah satu
//jika menggunakan esp32
//#include <WiFi.h>
//jika menggunakan esp8266

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// Wi-Fi configuration
#define WIFI_SSID "mama"
#define WIFI_PASSWORD "mama1234"
#define BOT_TOKEN "6215118145:AAGMI-5kmEwlU2ZpM9e2gFBqB1tH7XxXH1E"

const int moisturePin = A0;
const int pumpPin = D5;
const int dryThreshold = 300;  // Dry threshold value
const int wetThreshold = 500;  // Wet threshold value
const int stabilityDelay = 2000;  // Stability delay for sensor (in milliseconds)
const unsigned long BOT_MTBS = 1000; // mean time between scan messages
int moistureValue;
String chatId;

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime;

LiquidCrystal_I2C lcd(0x27, 16, 2); 

void handleNewMessages(int numNewMessages)
{
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = bot.messages[i].chat_id;
    chatId = chat_id;
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "")
      from_name = "Guest";

    if (text == "/send_test_action")
    {
      bot.sendChatAction(chat_id, "typing");
      delay(4000);
      bot.sendMessage(chat_id, "Did you see the action message?");
    }

    if (text == "/start")
    {
      String welcome = "Welcome to Universal Arduino Telegram Bot library, " + from_name + ".\n";
      welcome += "This is Chat Action Bot example.\n\n";
      welcome += "/status : check parameter\n";
      welcome += "/on_pompa : menghidupkan pompa\n";
      welcome += "/off_pompa : mematikan pompa\n";
      bot.sendMessage(chat_id, welcome);
    }

    else if (text == "/check_pH") {
      String response = "Kelembaban saat ini: ";
      bot.sendMessage(chat_id, response);
      sendTelegramStatus(moistureValue);
    }

    else if (text == "/on_pompa") {
      digitalWrite(pumpPin, LOW);
      String response = "Pump turned on. Tanaman disiram.";
      digitalWrite(pumpPin, LOW);
    }

    else if (text == "/off_pompa") {
      digitalWrite(pumpPin, HIGH);
      String response = "Pump turned off. Tanaman selesai disiram.";
      digitalWrite(pumpPin, HIGH);
    }

  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  
  secured_client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());
  configTime(0, 0, "pool.ntp.org");
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now);

  //check inisialisai dilaptop saya bisa pakek init
  lcd.init(); 
  //lcd.begin();
  lcd.backlight();   // Enable the backlight
  lcd.setCursor(0, 0);
  lcd.print("Test");
  lcd.setCursor(0, 1);
  lcd.print("Asd");


}

void loop() {
   moistureValue = analogRead(moisturePin);

  if (moistureValue >= wetThreshold) {
    lcd.setCursor(0, 0);
    lcd.print("Tanah Basah"+ String(moistureValue));
    lcd.setCursor(0, 1);
    lcd.print("Pump Off " );
    sendTelegramStatus(moistureValue);
  } else if (moistureValue <= dryThreshold) {
    lcd.setCursor(0, 0);
    lcd.print("Tanah Kering  " );
    lcd.setCursor(0, 1);
    lcd.print("Pump ON " + String(moistureValue));
    sendTelegramStatus(moistureValue);
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Kelembaban Tanah ");
    lcd.setCursor(0, 1);
    lcd.print("Saat ini: " + String(moistureValue));
  }
  //delay(stabilityDelay);  // Delay for sensor stability


  if (millis() - bot_lasttime > BOT_MTBS)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages)
    {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    bot_lasttime = millis();
  }

}

bool sendTelegramMessage(String message) {
  return bot.sendMessage(chatId, message);
}

void sendTelegramStatus(int moistureValue) {
  String statusMessage = "Moisture level: " + String(moistureValue);
  sendTelegramMessage(statusMessage);
}
