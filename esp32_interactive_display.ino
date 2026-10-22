/**
 * ESP32 Smart Interactive Display & Telegram Bot
 * 
 * Description: A capacitive-touch activated smart display that shows randomized 
 * text and bitmap graphics on an SSD1306 OLED. Integrates with the Telegram API 
 * via WiFi to receive live messages and trigger hidden remote animations.
 * 
 * Hardware: ESP32, 128x64 I2C OLED Display, Copper foil (Capacitive Touch)
 */

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- NETWORK & TELEGRAM CONFIGURATION ---
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* botToken = "YOUR_TELEGRAM_BOT_TOKEN";

WiFiClientSecure secured_client;
UniversalTelegramBot bot(botToken, secured_client);

unsigned long lastTimeBotRan;
const int botRequestDelay = 3000; // Check Telegram every 3 seconds

// --- OLED CONFIGURATION ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- HARDWARE CONFIGURATION ---
#define TOUCH_PIN 4
#define TOUCH_THRESHOLD 900 // Calibrated capacitance threshold

// --- PLACEHOLDER BITMAPS (64x64) ---
// Replace these arrays with actual generated PROGMEM hex data
const unsigned char PROGMEM bunny_normal[] = { /* Hex Data Here */ };
const unsigned char PROGMEM bunny_smiley[] = { /* Hex Data Here */ };
const unsigned char PROGMEM pic_1[] = { /* Hex Data Here */ };
const unsigned char PROGMEM pic_2[] = { /* Hex Data Here */ };
const unsigned char PROGMEM frame_1[] = { /* Hex Data Here */ };
const unsigned char PROGMEM frame_2[] = { /* Hex Data Here */ };

const int totalPictures = 2;
const unsigned char* myPictures[totalPictures] = { pic_1, pic_2 };

// --- DATA STRUCTURES ---
struct Sentence {
  String text;
  int size;
};

// Example dataset
const int totalSentences = 3;
Sentence mySentences[totalSentences] = {
  {"Hello!", 2},
  {"Have a great day", 1},
  {"Keep coding", 2}
};

int sentenceOrder[totalSentences];
int currentSentence = 0;
bool lastWasPicture = false;

// --- UTILITY FUNCTIONS ---
void shuffleSentences() {
  for (int i = 0; i < totalSentences; i++) {
    int randomIndex = random(0, totalSentences);
    int temp = sentenceOrder[i];
    sentenceOrder[i] = sentenceOrder[randomIndex];
    sentenceOrder[randomIndex] = temp;
  }
}

void setup() {
  Serial.begin(115200);

  // 1. Initialize Display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Freeze if display fails
  }
  display.clearDisplay();
  display.display();

  // 2. Initialize Networking
  Serial.print("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  secured_client.setInsecure(); // Bypass SSL certificate validation for Telegram API
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWi-Fi Connected!");

  // 3. System Prep
  randomSeed(esp_random()); // Hardware RNG for true randomness
  
  // Populate initial sequence array (0 to totalSentences-1)
  for (int i = 0; i < totalSentences; i++) {
    sentenceOrder[i] = i;
  }
  shuffleSentences();

  // 4. Flush pending Telegram messages on boot
  Serial.println("Clearing old Telegram messages...");
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  while (numNewMessages > 0) {
    bot.last_message_received = bot.messages[numNewMessages - 1].update_id;
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
  Serial.println("System Ready.");
}

void loop() {
  // ==========================================
  // ASYNCHRONOUS TELEGRAM CHECKER
  // ==========================================
  if (millis() - lastTimeBotRan > botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    
    while (numNewMessages > 0) {
      for (int i = 0; i < numNewMessages; i++) {
        String liveMessage = bot.messages[i].text;
        
        // --- HIDDEN COMMAND: ANIMATION TRIGGER ---
        if (liveMessage.indexOf("/boos") != -1) {
          Serial.println("Triggering animation sequence...");
          for (int loopCount = 0; loopCount < 6; loopCount++) {
            
            // Frame 1
            display.clearDisplay();
            display.drawBitmap(32, 0, frame_1, 64, 64, SSD1306_WHITE);
            display.display();
            delay(350); 
            yield(); // Feed hardware watchdog timer

            // Frame 2
            display.clearDisplay();
            display.drawBitmap(32, 0, frame_2, 64, 64, SSD1306_WHITE);
            display.display();
            delay(350); 
            yield(); 
          }
          delay(3000); // Hold final frame
        }
        // --- NORMAL LIVE MESSAGE ---
        else {
          display.clearDisplay();
          display.setTextSize(2); 
          display.setTextColor(SSD1306_WHITE);
          
          // Center dynamic text
          int16_t x1, y1;
          uint16_t w, h;
          display.getTextBounds(liveMessage, 0, 0, &x1, &y1, &w, &h);
          
          int startX = (SCREEN_WIDTH - w) / 2;
          int startY = (SCREEN_HEIGHT - h) / 2;
          if (startX < 0) startX = 0; 
          
          display.setCursor(startX, startY);
          display.println(liveMessage);
          display.display();
          
          delay(10000); // Display live message for 10 seconds
        }
      }
      
      // Acknowledge processed messages
      bot.last_message_received = bot.messages[numNewMessages - 1].update_id;
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }

  // ==========================================
  // CAPACITIVE TOUCH UI LOGIC
  // ==========================================
  int touchValue = touchRead(TOUCH_PIN);
  
  // --- DEFAULT STATE ---
  if(touchValue >= TOUCH_THRESHOLD) {
    display.clearDisplay();
    display.drawBitmap(32, 0, bunny_normal, 64, 64, SSD1306_WHITE);
    display.display();
  }
  
  // --- TRIGGERED STATE ---
  if(touchValue < TOUCH_THRESHOLD) {
    
    // 1. Initial reaction frame
    display.clearDisplay();
    display.drawBitmap(32, 0, bunny_smiley, 64, 64, SSD1306_WHITE);
    display.display();
    delay(800); 
    
    // 2. Content Selection Engine (Anti-Repeat Logic)
    int contentType = random(0, 2); 
    
    // Veto consecutive pictures
    if (contentType == 1 && lastWasPicture == true) {
      contentType = 0; 
    }
    lastWasPicture = (contentType == 1);
    
    display.clearDisplay();
    
    // Render Text
    if (contentType == 0) {
      int indexToPlay = sentenceOrder[currentSentence]; 
      String textToPrint = mySentences[indexToPlay].text;
      int textSize = mySentences[indexToPlay].size;
      
      display.setTextSize(textSize); 
      display.setTextColor(SSD1306_WHITE);
      
      int16_t x1, y1;
      uint16_t w, h;
      display.getTextBounds(textToPrint, 0, 0, &x1, &y1, &w, &h);
      
      int startX = (SCREEN_WIDTH - w) / 2;
      int startY = (SCREEN_HEIGHT - h) / 2;
      if (startX < 0) startX = 0; 
      
      display.setCursor(startX, startY);
      display.println(textToPrint);
      
      // Advance and re-shuffle if end of array is reached
      currentSentence++; 
      if(currentSentence >= totalSentences) {
        shuffleSentences();
        currentSentence = 0;
      }
    }
    // Render Bitmap
    else if (contentType == 1) {
      int randomIndex = random(0, totalPictures);
      display.drawBitmap(32, 0, myPictures[randomIndex], 64, 64, SSD1306_WHITE); 
    }
    
    display.display();
    
    // 3. Debounce: wait for touch release
    while(touchRead(TOUCH_PIN) < TOUCH_THRESHOLD) {
      delay(10); 
    }
    
    // 4. Skippable delay (Interruptible by another touch)
    unsigned long startTime = millis();
    while (millis() - startTime < 8000) {
      if (touchRead(TOUCH_PIN) < TOUCH_THRESHOLD) {
        break; 
      }
      delay(10); 
    }
  }
}
