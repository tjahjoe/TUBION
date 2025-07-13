#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>
#include <WiFiManager.h>
#include <FS.h>
#include <LittleFS.h>

#define LED_RED_PIN 4
#define LED_YELLOW_PIN 18
#define LED_BLUE_PIN 5
#define LED_GREEN_PIN 19

#define BUTTON_RED_PIN 27
#define BUTTON_YELLOW_PIN 12
#define BUTTON_BLUE_PIN 14
#define BUTTON_GREEN_PIN 13

#define SERVO_PIN 21

#define RELAY_FAN_PIN 23
#define RELAY_MIST_PIN 22
#define RELAY_PUMP_OUT_PIN 25
#define RELAY_PUMP_IN_PIN 26

#define MQ_PIN 35
#define MS_PIN 34
#define TGS_PIN 32

#define RXD2 16
#define TXD2 17

#define WIFI_CONFIG_FILE "/wifi_config.txt"

// const char *ssid = "Grayhouse";
// const char *password = "pinturumah";

int timeDetection = 5;
Servo servo;

const String FLASK_API_ENDPOINT = "http://192.168.1.4:5000/insert/data";
String status = "CLEAN";

void setupButton() {
  pinMode(BUTTON_RED_PIN, INPUT_PULLUP);
  pinMode(BUTTON_YELLOW_PIN, INPUT_PULLUP);
  pinMode(BUTTON_BLUE_PIN, INPUT_PULLUP);
  pinMode(BUTTON_GREEN_PIN, INPUT_PULLUP);
}

void setupRelay() {
  pinMode(RELAY_MIST_PIN, OUTPUT);
  pinMode(RELAY_PUMP_OUT_PIN, OUTPUT);
  pinMode(RELAY_PUMP_IN_PIN, OUTPUT);
  pinMode(RELAY_FAN_PIN, OUTPUT);

  digitalWrite(RELAY_MIST_PIN, HIGH);
  digitalWrite(RELAY_PUMP_OUT_PIN, HIGH);
  digitalWrite(RELAY_PUMP_IN_PIN, HIGH);
  digitalWrite(RELAY_FAN_PIN, HIGH);
}

void setupLed() {
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_YELLOW_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);

  digitalWrite(LED_RED_PIN, LOW);
  digitalWrite(LED_YELLOW_PIN, LOW);
  digitalWrite(LED_BLUE_PIN, LOW);
  digitalWrite(LED_GREEN_PIN, LOW);
}

void setupSensor(){
  pinMode(MQ_PIN, INPUT);
  pinMode(MS_PIN, INPUT);
  pinMode(TGS_PIN, INPUT);
}

void initLittleFS() {
  if (!LittleFS.begin(true)) {
    Serial.println(F("ERROR: LittleFS Mount Failed"));
  }
}

void saveWiFiCredentials(const String &ssid, const String &password) {
  File file = LittleFS.open(WIFI_CONFIG_FILE, "w");
  if (!file) return;
  file.println(ssid);
  file.println(password);
  file.close();
}

bool loadWiFiCredentials(String &ssid, String &password) {
  File file = LittleFS.open(WIFI_CONFIG_FILE, "r");
  if (!file) return false;
  ssid = file.readStringUntil('\n');
  ssid.trim();
  password = file.readStringUntil('\n');
  password.trim();
  file.close();
  return (ssid.length() > 0);
}

void saveWiFiManagerParamsCallback() {
  saveWiFiCredentials(WiFi.SSID(), WiFi.psk());
}

void deleteWiFiConfigFile() {
  if (LittleFS.exists(WIFI_CONFIG_FILE)) {
    LittleFS.remove(WIFI_CONFIG_FILE);
    Serial.println("wifi_config.txt deleted.");
  } else {
    Serial.println("wifi_config.txt not found.");
  }
}

void setupWiFi() {
  initLittleFS();

  String ssid, password;
  bool credentialsLoaded = loadWiFiCredentials(ssid, password);

  WiFiManager wm;
  wm.setSaveConfigCallback(saveWiFiManagerParamsCallback);

  if (credentialsLoaded) {
    WiFi.begin(ssid.c_str(), password.c_str());

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
      delay(500);
      Serial.print(F("."));
    }
    Serial.println();

    if (WiFi.status() != WL_CONNECTED) {
      if (!wm.autoConnect("TUBION", "12345678")) {
        ESP.restart();
      }
    }
  } else {
    if (!wm.autoConnect("TUBION", "12345678")) {
      ESP.restart();
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  setupWiFi();
  setupLed();
  setupButton();
  setupRelay();
  setupSensor();
  servo.attach(SERVO_PIN);
}

void sendDataToAnotherModule(int mq, int ms, int tgs){
    Serial2.print(mq);
    Serial2.print(",");
    Serial2.print(ms);
    Serial2.print(",");
    Serial2.println(tgs);
}

void activateDetection() {
  digitalWrite(LED_RED_PIN, HIGH);

  Serial2.println("DETECTION");

  digitalWrite(RELAY_PUMP_IN_PIN, LOW);
  delay(10000);
  digitalWrite(RELAY_PUMP_IN_PIN, HIGH);

  for (int i = 0; i < timeDetection; i++) {
    int mq = analogRead(MQ_PIN);
    int ms = analogRead(MS_PIN);
    int tgs = analogRead(TGS_PIN);

    // Serial.print("MQ135: ");
    // Serial.println(mq);

    sendDataToAnotherModule(mq, ms, tgs);
    sendDataToServer(mq, ms, tgs);
    delay(1000);
  }

  Serial2.println("STOP");

  digitalWrite(LED_RED_PIN, LOW);
}

void activateDry() {
  digitalWrite(LED_YELLOW_PIN, HIGH);

  Serial2.println("DRY");

  digitalWrite(RELAY_FAN_PIN, LOW);
  servo.write(0);
  delay(1000);
  servo.write(90);
  delay(10000);
  servo.write(180);
  delay(1000);
  servo.write(90);
  digitalWrite(RELAY_FAN_PIN, HIGH);

  Serial2.println("STOP");

  digitalWrite(LED_YELLOW_PIN, LOW);
}

void activateMist() {
  digitalWrite(LED_BLUE_PIN, HIGH);

  Serial2.println("CLEAN");

  digitalWrite(RELAY_MIST_PIN, LOW);
  delay(10000);
  digitalWrite(RELAY_MIST_PIN, HIGH);

  Serial2.println("STOP");

  digitalWrite(LED_BLUE_PIN, LOW);
}

void activatePumpOut() {
  digitalWrite(LED_GREEN_PIN, HIGH);

  Serial2.println("OUT");

  digitalWrite(RELAY_PUMP_OUT_PIN, LOW);
  delay(10000);
  digitalWrite(RELAY_PUMP_OUT_PIN, HIGH);

  Serial2.println("STOP");

  digitalWrite(LED_GREEN_PIN, LOW);
}

bool sendDataToServer(int mq, int ms, int tgs) {
  if (WiFi.status() != WL_CONNECTED) return false;

  HTTPClient http;
  http.begin(FLASK_API_ENDPOINT.c_str());
  http.addHeader(F("Content-Type"), F("application/json"));

  StaticJsonDocument<200> doc;
  doc["mq"] = mq;
  doc["ms"] = ms;
  doc["tgs"] = tgs;

  String payload;
  serializeJson(doc, payload);

  int httpResponseCode = http.POST(payload);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.print(F("Flask API Response: "));
    Serial.println(response);
    http.end();
    return true;
  } else {
    Serial.print(F("Flask API Error: "));
    Serial.println(httpResponseCode);
    http.end();
    return false;
  }
}

void loop() {
  if (digitalRead(BUTTON_RED_PIN) == LOW && status == "CLEAN") {
    activateDetection();
    status = "DIRT";
  } else if (digitalRead(BUTTON_YELLOW_PIN) == LOW && status == "CLEAN") {
    activateDry();
  } else if (digitalRead(BUTTON_BLUE_PIN) == LOW) {
    activateMist();
    status = "CLEAN";
  } else if (digitalRead(BUTTON_GREEN_PIN) == LOW && status == "CLEAN") {
    activatePumpOut();
  }
  // activateDetection();
  // activateDry();
  // activateMist();
  // activatePumpOut();
}
