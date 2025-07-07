#include <ESP32Servo.h>

#define SERVO_PIN 18
#define RELAY_FAN_PIN 19

#define RXD2 16
#define TXD2 17

Servo servo;
String command = "";

void setupRelay() {
  pinMode(RELAY_FAN_PIN, OUTPUT);
  digitalWrite(RELAY_FAN_PIN, HIGH);
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  servo.attach(SERVO_PIN);
  setupRelay();
}

void getCommand() {
  if (Serial2.available()) {
    command = Serial2.readStringUntil('\n');
    command.trim();
    Serial.println("Received command: " + command);
  }
}

void activateDry() {
  digitalWrite(RELAY_FAN_PIN, LOW);
  servo.write(0);
  delay(1000);
  servo.write(90);
  delay(10000);
  servo.write(180);
  delay(1000);
  servo.write(90);
  digitalWrite(RELAY_FAN_PIN, HIGH);
}

void loop() {
  getCommand();
  if (command == "DRY") {
    activateDry();
    command = "";
  }
}
