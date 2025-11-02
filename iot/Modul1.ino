#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>
#include <WiFiManager.h>
#include <FS.h>
#include <LittleFS.h>

#define LED_RED_PIN 4
#define LED_YELLOW_PIN 5
#define LED_BLUE_PIN 19
#define LED_GREEN_PIN 18

#define BUTTON_RED_PIN 13
#define BUTTON_YELLOW_PIN 12
#define BUTTON_BLUE_PIN 27
#define BUTTON_GREEN_PIN 14

#define SERVO_PIN 21

#define RELAY_FAN_PIN 22
#define RELAY_MIST_PIN 23
#define RELAY_PUMP_OUT_PIN 25
#define RELAY_PUMP_IN_PIN 26

#define MQ_PIN 35
#define MS_PIN 34
#define TGS_PIN 32

#define RXD2 16
#define TXD2 17

#define WIFI_CONFIG_FILE "/wifi_config.txt"
const String FLASK_API_ENDPOINT = "https://api-tubion.vercel.app/insert/data";

Servo servo;

const int timeDetection = 60;
const int numReadings = 5;
int validReadings = 0;
int readIndex = 0;

const float VTGS = 5.0;
const int RLTGS = 10;
const float mTGS = -0.160;
const float bTGS = 1.368;
const float RoTGS = 1.0;
float TGSReadings[numReadings];
float TGSSend[timeDetection];
float TGSTotal = 0;

const float VMQ = 5.0;
const int RLMQ = 10;
const float mMQ = -0.417;
const float bMQ = -0.858;
const float RoMQ = 7.0;
float MQReadings[numReadings];
float MQSend[timeDetection];
float MQTotal = 0;

const float VMS = 5.0;
const int RLMS = 2;
const float mMS = -0.223;
const float bMS = -0.227;
const float RoMS = 1.0;
float MSReadings[numReadings];
float MSSend[timeDetection];
float MSTotal = 0;

String status = "CLEAN";
String id;

void setupButton()
{
  pinMode(BUTTON_RED_PIN, INPUT_PULLUP);
  pinMode(BUTTON_YELLOW_PIN, INPUT_PULLUP);
  pinMode(BUTTON_BLUE_PIN, INPUT_PULLUP);
  pinMode(BUTTON_GREEN_PIN, INPUT_PULLUP);
}

void setupRelay()
{
  pinMode(RELAY_MIST_PIN, OUTPUT);
  pinMode(RELAY_PUMP_OUT_PIN, OUTPUT);
  pinMode(RELAY_PUMP_IN_PIN, OUTPUT);
  pinMode(RELAY_FAN_PIN, OUTPUT);

  digitalWrite(RELAY_MIST_PIN, HIGH);
  digitalWrite(RELAY_PUMP_OUT_PIN, HIGH);
  digitalWrite(RELAY_PUMP_IN_PIN, HIGH);
  digitalWrite(RELAY_FAN_PIN, HIGH);
}

void setupLed()
{
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_YELLOW_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);

  digitalWrite(LED_RED_PIN, LOW);
  digitalWrite(LED_YELLOW_PIN, LOW);
  digitalWrite(LED_BLUE_PIN, LOW);
  digitalWrite(LED_GREEN_PIN, LOW);
}

void setupSensor()
{
  pinMode(MQ_PIN, INPUT);
  pinMode(MS_PIN, INPUT);
  pinMode(TGS_PIN, INPUT);
}

void initLittleFS()
{
  if (!LittleFS.begin(true))
  {
    Serial.println(F("ERROR: LittleFS Mount Failed"));
  }
}

void saveWiFiCredentials(const String &ssid, const String &password)
{
  File file = LittleFS.open(WIFI_CONFIG_FILE, "w");
  if (!file)
    return;
  file.println(ssid);
  file.println(password);
  file.close();
}

bool loadWiFiCredentials(String &ssid, String &password)
{
  File file = LittleFS.open(WIFI_CONFIG_FILE, "r");
  if (!file)
    return false;
  ssid = file.readStringUntil('\n');
  ssid.trim();
  password = file.readStringUntil('\n');
  password.trim();
  file.close();
  return (ssid.length() > 0);
}

void saveWiFiManagerParamsCallback()
{
  saveWiFiCredentials(WiFi.SSID(), WiFi.psk());
}

void deleteWiFiConfigFile()
{
  if (LittleFS.exists(WIFI_CONFIG_FILE))
  {
    LittleFS.remove(WIFI_CONFIG_FILE);
    Serial.println("wifi_config.txt deleted.");
  }
  else
  {
    Serial.println("wifi_config.txt not found.");
  }
}

void setupWiFi()
{
  initLittleFS();

  String ssid, password;
  bool credentialsLoaded = loadWiFiCredentials(ssid, password);

  WiFiManager wm;
  wm.setSaveConfigCallback(saveWiFiManagerParamsCallback);

  if (credentialsLoaded)
  {
    WiFi.begin(ssid.c_str(), password.c_str());

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000)
    {
      delay(500);
      Serial.print(F("."));
    }
    Serial.println();

    if (WiFi.status() != WL_CONNECTED)
    {
      if (!wm.autoConnect("TUBION", "12345678"))
      {
        ESP.restart();
      }
    }
  }
  else
  {
    if (!wm.autoConnect("TUBION", "12345678"))
    {
      ESP.restart();
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  setupWiFi();
  setupLed();
  setupButton();
  setupRelay();
  setupSensor();
  servo.attach(SERVO_PIN);
}

void sendDataToAnotherModule(float mq, float ms, float tgs)
{
  Serial2.print(mq);
  Serial2.print(",");
  Serial2.print(ms);
  Serial2.print(",");
  Serial2.println(tgs);
}

float countPPM(float sensorValue, float voltage, int RL, float m, float b, float Ro, float *reading, float &total, int &readIndex, int &validReadings)
{
  float VRL = sensorValue * (voltage / 4095.0);
  float RS = (voltage / VRL - 1) * RL;
  float ratio = RS / Ro;
  float ppm = pow(10, ((log10(ratio) - b) / m));

  total = total - reading[readIndex];
  reading[readIndex] = ppm;
  total = total + reading[readIndex];

  if (validReadings < numReadings)
  {
    return total / validReadings;
  }
  else
  {
    return total / numReadings;
  }
}

void activateDetection()
{
  digitalWrite(LED_RED_PIN, HIGH);

  Serial2.println("DETECTION");

  // digitalWrite(RELAY_PUMP_IN_PIN, HIGH); 
  // digitalWrite(RELAY_PUMP_OUT_PIN, LOW);
  // delay(10000);
  // digitalWrite(RELAY_PUMP_IN_PIN, LOW);
  // digitalWrite(RELAY_PUMP_OUT_PIN, LOW);

  digitalWrite(RELAY_PUMP_IN_PIN, LOW);
  delay(10000);
  digitalWrite(RELAY_PUMP_IN_PIN, HIGH);

  for (int i = 0; i < timeDetection; i++)
  {

    float mq = analogRead(MQ_PIN);
    float ms = analogRead(MS_PIN);
    float tgs = analogRead(TGS_PIN);

    if (validReadings < numReadings)
      validReadings++;

    mq = countPPM(mq, VMQ, RLMQ, mMQ, bMQ, RoMQ, MQReadings, MQTotal, readIndex, validReadings);
    ms = countPPM(ms, VMS, RLMS, mMS, bMS, RoMS, MSReadings, MSTotal, readIndex, validReadings);
    tgs = countPPM(tgs, VTGS, RLTGS, mTGS, bTGS, RoTGS, TGSReadings, TGSTotal, readIndex, validReadings);

    readIndex = (readIndex + 1) % numReadings;

    sendDataToAnotherModule(mq, ms, tgs);

    MQSend[i] = mq;
    MSSend[i] = ms;
    TGSSend[i] = tgs;
    delay(1000);
  }

  sendDataToServer(MQSend, MSSend, TGSSend);

  Serial2.println("STOP " + id);

  digitalWrite(LED_RED_PIN, LOW);
}

void activateDry()
{
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

void activateMist()
{
  digitalWrite(LED_BLUE_PIN, HIGH);

  Serial2.println("CLEAN");

  digitalWrite(RELAY_MIST_PIN, LOW);
  delay(10000);
  digitalWrite(RELAY_MIST_PIN, HIGH);

  Serial2.println("STOP");

  digitalWrite(LED_BLUE_PIN, LOW);
}

void activatePumpOut()
{
  digitalWrite(LED_GREEN_PIN, HIGH);

  Serial2.println("OUT");

  // digitalWrite(RELAY_PUMP_IN_PIN, LOW);
  // digitalWrite(RELAY_PUMP_OUT_PIN, HIGH); 
  // delay(10000);
  // digitalWrite(RELAY_PUMP_IN_PIN, LOW);
  // digitalWrite(RELAY_PUMP_OUT_PIN, LOW);

  digitalWrite(RELAY_PUMP_OUT_PIN, LOW);
  delay(10000);
  digitalWrite(RELAY_PUMP_OUT_PIN, HIGH);

  Serial2.println("STOP");

  digitalWrite(LED_GREEN_PIN, LOW);
}

bool sendDataToServer(float mqArr[], float msArr[], float tgsArr[])
{
  if (WiFi.status() != WL_CONNECTED)
    return false;

  HTTPClient http;
  http.begin(FLASK_API_ENDPOINT.c_str());
  http.addHeader(F("Content-Type"), F("application/json"));

  StaticJsonDocument<8192> doc;

  JsonArray mqArray = doc.createNestedArray("mq");
  JsonArray msArray = doc.createNestedArray("ms");
  JsonArray tgsArray = doc.createNestedArray("tgs");

  for (int i = 0; i < timeDetection; i++)
  {
    mqArray.add(mqArr[i]);
    msArray.add(msArr[i]);
    tgsArray.add(tgsArr[i]);
  }

  String payload;
  serializeJson(doc, payload);

  int httpResponseCode = http.POST(payload);

  if (httpResponseCode > 0)
  {
    String response = http.getString();
    StaticJsonDocument<256> respDoc;
    DeserializationError error = deserializeJson(respDoc, response);

    if (!error)
    {
      id = respDoc["id"].as<String>();

      http.end();
      return true;
    }
    else
    {
      Serial.print("JSON Parse error: ");
      Serial.println(error.c_str());
      http.end();
      return false;
    }
  }
  else
  {
    Serial.print(F("Flask API Error: "));
    Serial.println(httpResponseCode);
    http.end();
    return false;
  }
}

void loop()
{
  if (digitalRead(BUTTON_RED_PIN) == LOW && status == "CLEAN")
  {
    activateDetection();
    status = "DIRT";
  }
  else if (digitalRead(BUTTON_YELLOW_PIN) == LOW && status == "CLEAN")
  {
    activateDry();
  }
  else if (digitalRead(BUTTON_BLUE_PIN) == LOW)
  {
    activateMist();
    status = "CLEAN";
  }
  else if (digitalRead(BUTTON_GREEN_PIN) == LOW && status == "CLEAN")
  {
    activatePumpOut();
  }
}
