# ESP32 Interactive Smart Display & Telegram Bot

## Overview
A capacitive-touch activated smart display built on the ESP32 platform. This project renders dynamic, randomized UI elements (text and PROGMEM bitmaps) on an SSD1306 OLED screen. It features an asynchronous Wi-Fi integration with the Telegram API, allowing remote users to trigger custom hardware animations via live messages without blocking the main hardware loop.

## Features
* **Asynchronous API Polling:** Checks for Telegram updates (`/boos` command or live text) without triggering hardware watchdog resets.
* **Capacitive Touch UI:** Utilizes ESP32's native touch pins with calibrated debouncing and thresholding to trigger display states.
* **Smart Content Randomizer:** Implements an anti-repeat algorithm and hardware-seeded RNG (`esp_random()`) to ensure dynamic content delivery.
* **Non-Blocking Architecture:** Uses `millis()` timers and `yield()` state management to keep the UI responsive during background network tasks.

## Hardware Used
* ESP32 Development Board
* 0.96" SSD1306 OLED Display (I2C)
* Copper tape/wire (Capacitive sensor input)

## Software Dependencies
* `WiFi.h` & `WiFiClientSecure.h`
* `UniversalTelegramBot`
* `ArduinoJson`
* `Adafruit_GFX` & `Adafruit_SSD1306`

## Developer Note
*Note: Wi-Fi credentials and Telegram Bot API tokens have been removed from this public repository for security purposes. To deploy this code locally, replace the placeholder variables at the top of the sketch with your own credentials.*
