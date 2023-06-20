//komen salah satu
//jika menggunakan esp32
//#include <WiFi.h>
//jika menggunakan esp8266

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// Wi-Fi configuration
#define WIFI_SSID "asdasd123"
#define WIFI_PASSWORD "asdasd123"
#define BOT_TOKEN "6242609061:AAEMRqTuNYfCiIBfDP4UU3SKhMkK8kmiW5I"

const int moisturePin = A0;
const int pumpPin = 27;
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

    if (text == "/status") {
      sendTelegramStatus(moistureValue);
    }

    if (text == "/on_pompa") {
      digitalWrite(pumpPin, LOW);
    }

    if (text == "/off_pompa") {
      digitalWrite(pumpPin, HIGH);
    }

  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  
  configTime(0, 0, "pool.ntp.org");
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

}

void loop() {
   moistureValue = analogRead(moisturePin);

  if (moistureValue <= wetThreshold) {
    digitalWrite(pumpPin, LOW);  // Turn off the pump
    Serial.println("Moisture level: Wet");
    sendTelegramMessage("Moisture level: Wet");
  } else if (moistureValue >= dryThreshold) {
    digitalWrite(pumpPin, HIGH);  // Turn on the pump
    Serial.println("Moisture level: Dry");
    sendTelegramMessage("Moisture level: Dry");
  }
  delay(stabilityDelay);  // Delay for sensor stability


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
