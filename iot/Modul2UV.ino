#include <TFT_eSPI.h>
#include <SPI.h>

// #define TFT_CS   33 
// #define TFT_DC   18  //pin in the range 0-31
// #define TFT_RST  32  // Reset pin, toggles on startup

// #define TFT_WR   19  //must use a pin in the range 0-31
// #define TFT_RD   23  // Read strobe control pin

// #define TFT_D0   13  // Must use pins in the range 0-31
// #define TFT_D1   12 
// #define TFT_D2   14
// #define TFT_D3   27 
// #define TFT_D4   26
// #define TFT_D5   25 
// #define TFT_D6   21
// #define TFT_D7   22

// #define TFT_INVERSION_ON

// #define TFT_RGB_ORDER TFT_RGB

#define RXD2 16
#define TXD2 17

TFT_eSPI tft = TFT_eSPI();

String command = "";
String status = "CLEAN";

void defaultText()
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString("READY", tft.width() / 2, tft.height() / 2 - 20, 4);
  tft.drawCentreString(status, tft.width() / 2, tft.height() / 2 + 10, 4);
}

void setup()
{
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  tft.init();
  tft.setRotation(1);
  defaultText();
}

void stopText(String text)
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString("STOPPED", tft.width() / 2, tft.height() / 2 - 20, 4);
  tft.drawCentreString(text, tft.width() / 2, tft.height() / 2 + 10, 4);
  delay(3000);
}

void showId(String id)
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString("ID", tft.width() / 2, tft.height() / 2 - 20, 4);
  tft.drawCentreString(id, tft.width() / 2, tft.height() / 2 + 10, 4);
  delay(3000);
}

void getCommand()
{
  if (Serial2.available())
  {
    command = Serial2.readStringUntil('\n');
    command.trim();
    Serial.println("Command: " + command);
  }
}

void getData()
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString("DETECTION", tft.width() / 2, tft.height() / 2 - 20, 4);

  while (Serial2.available())
    Serial2.read();

  while (true)
  {
    if (Serial2.available())
    {
      String receivedData = Serial2.readStringUntil('\n');
      receivedData.trim();

      for (int i = 0; i < receivedData.length(); i++)
      {
        if (!isPrintable(receivedData.charAt(i)))
        {
          receivedData.remove(i, 1);
          i--;
        }
      }

      if (receivedData.startsWith("STOP"))
      {
        String id = receivedData.substring(5);
        id.trim();
        showId(id);
        break;
      }

      int firstComma = receivedData.indexOf(',');
      int lastComma = receivedData.lastIndexOf(',');

      if (firstComma > 0 && lastComma > firstComma)
      {
        float mq = receivedData.substring(0, firstComma).toFloat();
        float ms = receivedData.substring(firstComma + 1, lastComma).toFloat();
        float tgs = receivedData.substring(lastComma + 1).toFloat();

        displayData(mq, ms, tgs);
      }
    }
  }
}

void displayData(float mq, float ms, float tgs)
{
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

void displayClean()
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString("CLEANING", tft.width() / 2, tft.height() / 2 - 10, 4);

  while (Serial2.available())
    Serial2.read();

  while (true)
  {
    if (Serial2.available())
    {
      String receivedData = Serial2.readStringUntil('\n');
      receivedData.trim();

      for (int i = 0; i < receivedData.length(); i++)
      {
        if (!isPrintable(receivedData.charAt(i)))
        {
          receivedData.remove(i, 1);
          i--;
        }
      }

      // Serial.println("Command: " + receivedData);

      if (receivedData == "STOP")
      {
        stopText("CLEANING MODE");
        break;
      }
    }
  }
}

void displayOut()
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString("OUT", tft.width() / 2, tft.height() / 2 - 10, 4);

  while (Serial2.available())
    Serial2.read();

  while (true)
  {
    if (Serial2.available())
    {
      String receivedData = Serial2.readStringUntil('\n');
      receivedData.trim();

      for (int i = 0; i < receivedData.length(); i++)
      {
        if (!isPrintable(receivedData.charAt(i)))
        {
          receivedData.remove(i, 1);
          i--;
        }
      }

      // Serial.println("Command: " + receivedData);

      if (receivedData == "STOP")
      {
        stopText("OUT THE AIR");
        break;
      }
    }
  }
}

void loop()
{
  getCommand();

  if (command == "DETECTION")
  {
    getData();
    status = "DIRT";
    defaultText();
    command = "";
  }
  else if (command == "CLEAN")
  {
    displayClean();
    status = "CLEAN";
    defaultText();
    command = "";
  }
  else if (command == "OUT")
  {
    displayOut();
    defaultText();
    command = "";
  }
}
