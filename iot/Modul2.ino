#include <TFT_eSPI.h>
#include <SPI.h>

#define RXD2 16
#define TXD2 17

TFT_eSPI tft = TFT_eSPI();

String command = "";
String status = "CLEAN";

void defaultText() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString("READY", tft.width() / 2, tft.height() / 2 - 20, 4);
  tft.drawCentreString(status, tft.width() / 2, tft.height() / 2 + 10, 4);
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  tft.init();
  tft.setRotation(1);
  defaultText();
}

void stopText(String text) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString("STOPPED DETECTION MODE", tft.width() / 2, tft.height() / 2 - 10, 4);
  delay(3000);
}

void getCommand() {
  if (Serial2.available()) {
    command = Serial2.readStringUntil('\n');
    command.trim();
    Serial.println("Command: " + command);
  }
}

void getData() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString("DETECTION", tft.width() / 2, tft.height() / 2 - 20, 4);

  while (true) {
    if (Serial2.available()) {
      String receivedData = Serial2.readStringUntil('\n');
      receivedData.trim();

      if (receivedData == "STOP") {
        stopText("STOPPED DETECTION MODE");
        break;
      }

      int firstComma = receivedData.indexOf(',');
      int lastComma = receivedData.lastIndexOf(',');

      if (firstComma > 0 && lastComma > firstComma) {
        float mq = receivedData.substring(0, firstComma).toFloat();
        float ms = receivedData.substring(firstComma + 1, lastComma).toFloat();
        float tgs = receivedData.substring(lastComma + 1).toFloat();

        displayData(mq, ms, tgs);
      }
    }
  }
}

void displayData(float mq, float ms, float tgs) {
  int y = tft.height() / 2;
  int h = 40;
  int w = tft.width();
  int mq_x = w / 6;
  int ms_x = w / 2;
  int tgs_x = 5 * w / 6;

  tft.fillRect(0, y, w, h, TFT_BLACK);

  char buffer[30];

  sprintf(buffer, "MQ: %.2f", mq);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString(buffer, mq_x, tft.height() / 2 + 10, 2);

  sprintf(buffer, "MS: %.2f", ms);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString(buffer, ms_x, tft.height() / 2 + 10, 2);

  sprintf(buffer, "TGS: %.2f", tgs);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString(buffer, tgs_x, tft.height() / 2 + 10, 2);
}

void displayClean() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString("CLEANING", tft.width() / 2, tft.height() / 2 - 10, 4);
  while (true) {
    if (Serial2.available()) {
      String receivedData = Serial2.readStringUntil('\n');
      receivedData.trim();

      if (receivedData == "STOP") {
        stopText("STOPPED CLEANING MODE");
        break;
      }
    }
  }
}

void displayDry() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString("DRYING", tft.width() / 2, tft.height() / 2 - 10, 4);
  while (true) {
    if (Serial2.available()) {
      String receivedData = Serial2.readStringUntil('\n');
      receivedData.trim();

      if (receivedData == "STOP") {
        stopText("STOPPED DRYING MODE");
        break;
      }
    }
  }
}

void displayOut() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString("OUT", tft.width() / 2, tft.height() / 2 - 10, 4);
  while (true) {
    if (Serial2.available()) {
      String receivedData = Serial2.readStringUntil('\n');
      receivedData.trim();

      if (receivedData == "STOP") {
        stopText("BLOWING OUT THE AIR");
        break;
      }
    }
  }
}

void loop() {
  getCommand();

  if (command == "DETECTION") {
    getData();
    status = "DIRT";
    defaultText();
    command = "";
  } else if (command == "CLEAN") {
    displayClean();
    status = "CLEAN";
    defaultText();
    command = "";
  } else if (command == "DRY") {
    displayDry();
    defaultText();
    command = "";
  } else if (command == "OUT") {
    displayOut();
    defaultText();
    command = "";
  }

  //  for (int i = 0; i < 100; i++) {
  //   displayData(i, i+1, i+2);
  //   delay(1000);
  // }
}
