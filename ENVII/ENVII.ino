/*Please install the < Adafruit BMP280 Library > （https://github.com/adafruit/Adafruit_BMP280_Library）
   from the library manager before use.
  This code will display the temperature, humidity and air pressure information on the screen*/
#include <M5Atom.h>
#include <Wire.h>
#include "Adafruit_Sensor.h"
#include <Adafruit_BMP280.h>
#include "SHT3X.h"
#include "Ambient.h"

// 温度・湿度センサ
SHT3X sht30;

// 気圧センサ(BMP280)
Adafruit_BMP280 bme;

// Wifiクライアント
WiFiClient wifiClient;

// Wifi SSID
const char* ssid = "CHANGE ME";

// Wifi パスワード
const char* password = "CHANGE ME";

// Ambient
Ambient ambient;

// Ambient チャネルID
unsigned int channelId = 0;

// Ambient ライトキー
const char* writeKey = "CHANGE ME";

void setup() {
  // M5ライブラリの初期化
  M5.begin(true, false, true);
  Wire.begin(26, 32);

  // 気圧センサのチェック
  Serial.println(F("ENV Unit(SHT30 and BMP280) test..."));
  while (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
  }

  // チャネルIDとライトキーを指定してAmbientの初期化
  ambient.begin(channelId, writeKey, &wifiClient);
}

void loop() {
  // 変数を宣言
  float pressure = 0.0;
  float tmp = 0.0;
  float hum = 0.0;

  // 気圧を取得
  pressure = bme.readPressure();

  if (sht30.get() == 0) {
    // 温度を取得
    tmp = sht30.cTemp;

    // 湿度を取得
    hum = sht30.humidity;
  }
  Serial.printf("Temperature: %2.2f*C  Humidity: %0.2f%%  Pressure: %0.2fPa\r\n", tmp, hum, pressure);

  // debug LEDを点灯
  M5.dis.drawpix(0, 0xf00000);

  // Ambientにデータを送信
  sendAmbient(tmp, hum, pressure);

  // debug LEDを消灯
  M5.dis.drawpix(0, 0x000000);

  // スリープ
  delay(60000);
}

/**
    Ambientにデータを送信.
*/
void sendAmbient(float tmp, float hum, float pressure) {
  // Wifi接続確認
  while (WiFi.status() != WL_CONNECTED) {
    // Wifiに接続
    connectWifi();
  }

  // Ambientにデータを送信.
  ambient.set(1, tmp);
  ambient.set(2, hum);
  ambient.set(3, pressure);
  if (ambient.send()) {
    Serial.println("Ambient::send() success");
  } else {
    Serial.println("Ambient::send() failure");
  }
}

/**
   Wifiに接続.
*/
void connectWifi() {
  int count;

  // 前回接続時情報で接続する
  Serial.println("WiFi begin");
  WiFi.begin(ssid, password);
  count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    count++;
    Serial.print(".");
    delay(500);
    if (10 < count) {
      break;
    }
  }
  Serial.println("");

  // 未接続の場合にはSmartConfig待受
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_STA);
    WiFi.beginSmartConfig();

    Serial.println("Waiting for SmartConfig");
    count = 0;
    while (!WiFi.smartConfigDone()) {
      count++;
      delay(1000);
      Serial.print("#");
      // 30秒以上接続できなかったら抜ける
      if (30 < count) {
        Serial.println("");
        Serial.println("Reset");
        ESP.restart();
      }
    }

    // Wifi接続
    Serial.println("");
    Serial.println("Waiting for WiFi");
    count = 0;
    while (WiFi.status() != WL_CONNECTED) {
      count++;
      delay(1000);
      Serial.print(".");
      // 60秒以上接続できなかったら抜ける
      if (60 < count) {
        Serial.println("");
        Serial.println("Reset");
        ESP.restart();
      }
    }
    Serial.println("");
    Serial.println("WiFi Connected.");
  }
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}
