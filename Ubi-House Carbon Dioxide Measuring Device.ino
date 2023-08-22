#include <M5StickC.h>
#include "MHZ19.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define RX_PIN 36
#define TX_PIN 26
#define BAUDRATE 9600

#define LCD_MODE_DIGIT 0
#define LCD_MODE_GRAPH 1
#define BRIGHTNESS 8

const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";
const char* serverName = "http://echo.tokyoec.com/wp-json/wp/v2/posts/";
String serverPath = "";

MHZ19 myMHZ19;
HardwareSerial mySerial(1);

bool lcdOn = true;
int lcdMode = LCD_MODE_DIGIT;
bool ledOn = false;
bool ledValue = false;
unsigned long nextLedOperationTime = 0;
unsigned long getDataTimer = 0;

int history[160] = {};
int historyPos = 0;

void setup() {
  M5.begin();
  M5.Axp.ScreenBreath(BRIGHTNESS);

  Serial.begin(9600);
  mySerial.begin(BAUDRATE, SERIAL_8N1, RX_PIN, TX_PIN);
  myMHZ19.begin(mySerial);
  myMHZ19.autoCalibration(true);

  M5.Lcd.setRotation(1);
  render();

  pinMode(M5_LED, OUTPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  auto now = millis();

  M5.update();
  if ( M5.BtnA.wasPressed() ) {
    lcdMode = (lcdMode + 1) % 2;
    render();
  }
  if ( M5.BtnB.wasPressed() ) {
    lcdOn = !lcdOn;

    if (lcdOn) {
      render();
      M5.Axp.ScreenBreath(BRIGHTNESS);
    } else {
      M5.Axp.ScreenBreath(0);
    }
  }
  if ((ledOn || !ledValue) && nextLedOperationTime <= now) {
    ledValue = !ledValue;
    digitalWrite(M5_LED, ledValue);

    if (ledValue) {
      nextLedOperationTime = now + 15000;
    } else {
      nextLedOperationTime = now + 800;
    }
  }
  if (now - getDataTimer >= 10000) {
    int CO2 = myMHZ19.getCO2();
    int8_t temp = myMHZ19.getTemperature(false);

    Serial.print("CO2 (ppm): ");
    Serial.print(CO2);
    Serial.print(", Temperature (C): ");
    Serial.println(temp);

    ledOn = CO2 >= 1200;

    historyPos = (historyPos + 1) % (sizeof(history) / sizeof(int));
    history[historyPos] = CO2;
    render();

    getDataTimer = now;

    // Update to WordPress
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      StaticJsonDocument<200> doc;
      doc["title"] = "CO2 level";
      doc["status"] = "publish";
      doc["slug"] = "co2-level";
      doc["categories"] = 5;
      doc["date"] = "2023-07-20T00:00:00";
      doc["content"] = "Current CO2 level is: " + String(CO2);
      String output;
      serializeJson(doc, output);

      http.begin(serverName + serverPath);
      http.addHeader("Content-Type", "application/json");
      http.setAuthorization("username", "password");
      int httpResponseCode = http.POST(output);

      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.print("Status code: ");
        Serial.println(httpResponseCode);
        Serial.print("Response: ");
        Serial.println(response);
      } else {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
      }

      http.end();
    }
  }
}

void render() {
  if (!lcdOn) {
    return;
  }

  int height = M5.Lcd.height();
  int width = M5.Lcd.width();
  M5.Lcd.fillRect(0, 0, width, height, BLACK);

  switch (lcdMode) {
    case LCD_MODE_DIGIT:
      M5.Lcd.drawString("CO2 [ppm]", 12, 0, 2);
      M5.Lcd.drawRightString("    " + (String)history[historyPos], width, 24, 7);
      break;
    case LCD_MODE_GRAPH:
      int len = sizeof(history) / sizeof(int);
      for (int i = 0; i < len; i++) {
        auto value = max(0, history[(historyPos + 1 + i) % len] - 400);
        auto y = min(height, (int)(value * (height / 1200.0)));
        auto color = min(255, (int)(value * (255 / 1200.0)));
        M5.Lcd.drawLine(i, height - y, i, height, M5.Lcd.color565(255, 255 - color, 255 - color));
      }
      break;
  }
}
