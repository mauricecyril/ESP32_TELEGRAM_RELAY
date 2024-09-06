#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include "Arduino.h"
#include <ArduinoJson.h>
#include <UniversalTelegramBot.h>
#include <U8x8lib.h>

/***************************************
 *  Select Functions
 **************************************/
// Setup m5stick Pins
// #define LedPin 19
// #define IrPin 17
// #define BuzzerPin 26
#define BtnPin 35

// Setup Sensor Pins
//#define PIN13 13  // GPIO13 Not used
#define RELAY 25  // GPIO25 used to control the relay

/***************************** Variables For Screen ************************************/
// Setup U8x8 Screen on the m5stick
// U8x8 constructor for your display
U8X8_SH1107_64X128_4W_HW_SPI u8x8(14, /* dc=*/ 27, /* reset=*/ 33);

// Create a U8x8log object
U8X8LOG u8x8log;

// Define the dimension of the U8x8log window
#define U8LOG_WIDTH 8
#define U8LOG_HEIGHT 16

// Allocate static memory for the U8x8log window
uint8_t u8log_buffer[U8LOG_WIDTH*U8LOG_HEIGHT];


/***************************************
 *  WiFi 
 **************************************/
#define WIFI_SSID "REPLACE_WITH_YOUR_SSID"
#define WIFI_PASSWD "REPLACE_WITH_YOUR_PASSWORD"

// Setup Wifi Variables
WiFiMulti wifiMulti;
WiFiClientSecure clientTCP;


// Telegram Bot Token
#define BOT_TOKEN  "XXXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
#define ALLOWED_ID "XXXXXXXXXX"

// Setup Bot
UniversalTelegramBot bot(BOT_TOKEN, clientTCP);

const unsigned long BOT_MTBS = 1000; // mean time between scan messages
unsigned long bot_lasttime; // last time messages' scan has been done


// Send Toggle Function
void sendTOGGLE() {
  // Toggle High and Low Quickly 
  digitalWrite(25, HIGH);
  delay(300);
  digitalWrite(25, LOW);
  delay(300);
}

// Message Handler Function and Functions for other bot commands
void handleNewMessages(int numNewMessages) {
  Serial.print("handleNewMessages ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++) {
    Serial.println(bot.messages[i].from_id);
    if (bot.messages[i].from_id != ALLOWED_ID ) continue;
    
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";

    if (text == "/toggle") {
      sendTOGGLE();
      bot.sendMessage(chat_id, "Toggle Command Sent", "");
    } else if (text == "/high") {
      // Insert Logic Here
      digitalWrite(25, HIGH);
      delay(300);
      bot.sendMessage(chat_id, "Relay is set to High.", "");
    } else if (text == "/low") {
      // Insert Logic Here
      digitalWrite(25, LOW);
      delay(300);
      bot.sendMessage(chat_id, "Relay is set to Low.", "");
    } else if (text == "/ping") {
      bot.sendMessage(chat_id, "Pong.", "");
    } else if (text == "/start") {
      String welcome = "Welcome to the Garage Relay**, " + from_name + ".\n";
      welcome += "Use is restricted to the bot owner.\n\n";
      welcome += "/high : Switch the Relay to High\n";
      welcome += "/low : Switch the Relay to Low\n";
      welcome += "/toggle : Send the Toggle Command\n";
      welcome += "/ping : Check the bot status\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
  }
}





void setup() {
  // put your setup code here, to run once:

// Setup Relay PIN 25 to Output
pinMode(RELAY,OUTPUT);

// Setup Button Pin to Input
pinMode(BtnPin,INPUT);

// Configure WiFI
  wifiMulti.addAP(WIFI_SSID, WIFI_PASS);
  

// Startup U8x8
  u8x8.begin();
  
  // Set a suitable font. This font will be used for U8x8log
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  
  // Start U8x8log, connect to U8x8, set the dimension and assign the static memory
  u8x8log.begin(u8x8, U8LOG_WIDTH, U8LOG_HEIGHT, u8log_buffer);
  
  // Set the U8x8log redraw mode
  u8x8log.setRedrawMode(1);    // 0: Update screen with newline, 1: Update screen for every char  
  
  // Setup Serial
  Serial.begin(9600);



    // Display Start Message
    Serial.println((String)"Starting Relay");
    u8x8log.println((String)"Starting Relay");

    // Add root certificate for api.telegram.org
   clientTCP.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  // Connect to Telegram
    Serial.print("Connecting to Telegram");
    u8x8log.println("Connecting to Telegram");
   

  Serial.print("Connecting to WiFI...");
  while ((wifiMulti.run() != WL_CONNECTED)) { // wait for WiFi connection.
    delay(500);
    Serial.print(".");
  }
  Serial.println("OK");

  

   // Send Inital Boot message
    String initial = "Relay is Online and Awaiting Commands";
    bot.sendMessage(ALLOWED_ID, initial, "");

    // Display Connection Confirmation on Screen
    Serial.print("Connected to Telegram");
    u8x8log.println("Connected to Telegram");


    // Set Initial Screen
    u8x8log.println((String)"Listening for Command"); 
    Serial.println((String)"Listening for Command");
    delay(5000);
    
}

void loop() 
{
  if (millis() - bot_lasttime > BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      Serial.println("Response received");
      u8x8log.println((String)"Response received"); 
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    bot_lasttime = millis();
  }

  delay(10);
    // Clear the Screen
    u8x8.clearDisplay();

}

