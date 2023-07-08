#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

#define ssid "HUAWEI-76Cq"
#define pass "kn7h39Vd"
#define BOT_TOKEN "6242609061:AAEMRqTuNYfCiIBfDP4UU3SKhMkK8kmiW5I"



X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime;

#define pinSensor A0
#define pinPompa D8

int valueSensor;
int valuePompa;

int dryThreshold = 300;
int wetThreshold = 500;



const unsigned long BOT_MTBS = 1000; // mean time between scan messages
String chatId;

String keadaan;

int cnt = 0;
int timernotif = 5; //satuan detik

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  //inisialisasi serial monitor
  Serial.begin(9600);
  //inisialisasi relay sebagai output
  pinMode(pinPompa, OUTPUT);
  digitalWrite(pinPompa, LOW);
  //inisialisasi telegram
  secured_client.setTrustAnchors(&cert);
  //connect wifi
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(ssid);
  WiFi.begin(ssid, pass);
  //inisialisasi lcd
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("nama");
  lcd.setCursor(0, 1);
  lcd.print("nim");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  //check connected
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
  digitalWrite(pinPompa, LOW);

  delay(2000);
}

void loop() {
  valueSensor = analogRead(pinSensor);
  Serial.println(valueSensor);
  if (valueSensor >= wetThreshold) {
    lcd.setCursor(0, 0);
    lcd.print("Tanah Basah");
    lcd.setCursor(0, 1);
    lcd.print("Pump Off " + String(valueSensor));
    Serial.println("basah");
    digitalWrite(pinPompa, LOW);

  } else if (valueSensor <= dryThreshold) {
    lcd.setCursor(0, 0);
    lcd.print("Tanah Kering  " );
    lcd.setCursor(0, 1);
    lcd.print("Pump ON " + String(valueSensor));
    Serial.println("kering");
    digitalWrite(pinPompa, HIGH);

  } else {
    lcd.setCursor(0, 0);
    lcd.print("Kelembaban Tanah ");
    lcd.setCursor(0, 1);
    lcd.print("Saat ini: " + String(valueSensor));
    digitalWrite(pinPompa, LOW);
  }

  if (cnt == timernotif) {
    sendTelegramStatus(valueSensor);
    Serial.println("kirim notif");
    cnt = 0;
  }
  lcd.setCursor(0, 0);
  lcd.print("               ");
  lcd.setCursor(0, 1);
  lcd.print("                ");

  if (millis() - bot_lasttime > BOT_MTBS)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages)
    {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    Serial.print("detik ke :");
    Serial.println(cnt);

    cnt++;
    bot_lasttime = millis();
  }
}

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
      welcome += "/check_pH : check parameter\n";
      welcome += "/on_pompa : menghidupkan pompa\n";
      welcome += "/off_pompa : mematikan pompa\n";
      bot.sendMessage(chat_id, welcome);
    }

    else if (text == "/check_pH") {
      String response = "Kelembaban saat ini: ";
      bot.sendMessage(chat_id, response);
      sendTelegramStatus(valueSensor);
    }

    else if (text == "/on_pompa") {
      String response = "Pump turned on. Tanaman disiram.";
      bot.sendMessage(chat_id, response);
      digitalWrite(pinPompa, HIGH);
    }

    else if (text == "/off_pompa") {
      String response = "Pump turned off. Tanaman selesai disiram.";
      bot.sendMessage(chat_id, response);
      digitalWrite(pinPompa, LOW);
    }
  }
}

void sendTelegramStatus(int moistureValue) {
  String statusMessage = "Moisture level: " + String(moistureValue);
  sendTelegramMessage(statusMessage);
}

bool sendTelegramMessage(String message) {
  return bot.sendMessage(chatId, message);
}
