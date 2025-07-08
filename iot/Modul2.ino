#include <TFT_eSPI.h>
#include <SPI.h>

#define RXD2 16
#define TXD2 17

TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  tft.init();
  tft.setRotation(1);         
  tft.fillScreen(TFT_BLACK); 
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(2);

  tft.drawCentreString("Monitoring Sensor", tft.width() / 2, 10, 2);
}

void getCommand() {
  if (Serial2.available()) {
    String receivedData = Serial2.readStringUntil('\n');

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

void displayData(float mq, float ms, float tgs) {
  tft.fillRect(0, 40, tft.width(), tft.height() - 40, TFT_BLACK);

  int w = tft.width();
  int mq_x = w / 6;    
  int ms_x = w / 2;      
  int tgs_x = 5 * w / 6;  

  char buffer[30];

  sprintf(buffer, "MQ: %.2f", mq);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawCentreString(buffer, mq_x, 80, 2);

  sprintf(buffer, "MS: %.2f", ms);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawCentreString(buffer, ms_x, 80, 2);

  sprintf(buffer, "TGS: %.2f", tgs);
  tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
  tft.drawCentreString(buffer, tgs_x, 80, 2);
}

void loop() {
  getCommand();
}
