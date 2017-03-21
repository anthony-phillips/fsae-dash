#include <Adafruit_GFX.h>    // Core graphics library
#include "Adafruit_ILI9341.h" // Hardware-specific library
#include <SPI.h>
#include <SD.h>
#include "FreeSansBold24pt7b.h"
#include "FreeSansBold12pt7b.h"

#define TFT_DC 9
#define TFT_CS 10
#define SD_CS 23

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

#define bootColor ILI9341_BLACK //color of screen while booting

#define rpmRange1 3000 //under 3K RPM is green
#define rpmRange2 6000 //above 3K & under 6K yellow
#define rpmRange3 9000 //above 6K & under 9K orange, above this is red
#define rpmMax 11000 //sets 12K RPM to max rpm the tach will display

#define rpmRange1Color ILI9341_GREEN //color of RPM range 1
#define rpmRange2Color ILI9341_YELLOW //color of RPM range 2
#define rpmRange3Color ILI9341_ORANGE //color of RPM range 3
#define rpmRange4Color ILI9341_RED //color of RPM range 4

#define gearIndiColor ILI9341_DARKGREY //gear area color
#define speedIndiColor ILI9341_DARKGREY //speed area color
#define rpmBackColor ILI9341_BLACK //rpm background color
#define warnColor ILI9341_DARKGREY //warning area color
#define warnLightColor ILI9341_RED //warning light color

#define speedTextColor ILI9341_WHITE //color of text for speed
#define gearTextColor ILI9341_WHITE //color of text for gear

#define rpmBackHeight 100 //height of bar behind rpm //there are 12 segments
#define rpmHeight 96 //height of each rpm segment
#define rpmWidth 22 //width of each rpm segment
#define rpmLeftMargin 5 //left margin of first rpm segment
#define rpmRightMargin 2 //right margin of every rpm segment


#define screenWidth 320 
#define screenHeight 240

const int totalRatio[] = {1, 2, 3, 4, 5}; //rpm to wheel speed ratio for each gear


int lastRPM = 99; //used to detect if the data needs updated
int lastGear = 99;
int lastSpeed = 99;

void setup(void) {
  //Serial.begin(115200);
  tft.begin();
  tft.fillScreen(bootColor);
  SD.begin(SD_CS);
  tft.setRotation(3);
  for (int i = 0; i < rpmMax; i += 1000) {
    drawRPM(i);
  }
  drawGear(1);
  drawSpeed(0);
  drawWarn(false, false, false);
}

void loop() {
  //drive mode
  //read CAN bus now

  //demo
  for (int j = 1; j <= 5; j ++){
  for (int i = 7500; i < 11000; i += 100) {
    int rpm = i;
    int gear = j;
    int mph = totalRatio[gear - 1] * rpm / 1000;
   // mph = int(mph / 2) * 2; //only even speeds
    drawRPM(rpm);
    drawGear(gear);
    unsigned long startTime = millis();
    drawSpeed(mph);
    unsigned long endTime = millis();
    Serial.println(endTime - startTime);
    delay(16);
  }
}
delay(5000);
}

/////////////////////////////////////////////////////////////////////////////

void drawSpeed(int mph) { //only updates if the speed changed
  if (mph != lastSpeed) {
    lastSpeed = mph;
    tft.fillRect(160, 100, 160, 140, speedIndiColor); //speed indicator area
    int xOffset;
    if (mph < 10) //single digit
      xOffset = 215;
    if (mph >= 10) //double digit
      xOffset = 185;
    if (mph >= 100)  //3 digits and above replace with 99
      mph = 99;
    tft.setRotation(3);
    tft.setFont(&FreeSansBold24pt7b);
    tft.setCursor(xOffset, 190);
    tft.setTextColor(speedTextColor);
    tft.setTextSize(2);
    tft.println(String(mph));

    tft.setFont(&FreeSansBold12pt7b);
    tft.setCursor(215, 220);
    tft.setTextColor(speedTextColor);
    tft.setTextSize(1);
    tft.println("MPH");
  }
}

void drawGear(int gear) {
  if (gear != lastGear) { //only updates if the gear changed
    if (gear > 5 || gear < 1) //just in case gear is calculated wrong
      gear = 0;
    lastGear = gear;
     tft.setRotation(3);
    tft.fillRect(0, 100, 100, 140, gearIndiColor); //gear indicator area
    tft.setRotation(3);
    tft.setFont(&FreeSansBold24pt7b);
    tft.setCursor(25, 190);
    tft.setTextColor(gearTextColor);
    tft.setTextSize(2);
    tft.println(String(gear));

    tft.setFont(&FreeSansBold12pt7b);
    tft.setCursor(15, 220);
    tft.setTextColor(gearTextColor);
    tft.setTextSize(1);
    tft.println("GEAR");
  }
}

void drawRPM(int rpm) {
  int color; //don't ask why this is an int
  rpm = int(rpm / 1000) * 1000 - 1; //rounds rpm to nearest 1K
  if (rpm != lastRPM) { //only updates tach if rpm changed
    lastRPM = rpm;
    if (rpm < rpmRange1) {
      color = rpmRange1Color;
    } else if (rpm < rpmRange2) {
      color = rpmRange2Color;
    } else if (rpm < rpmRange3) {
      color = rpmRange3Color;
    } else {
      color = rpmRange4Color;
      if (rpm > rpmMax)
        rpm = rpmMax;
    }
    int xOffset = int(rpm / 1000) * (rpmWidth + rpmRightMargin) + rpmLeftMargin;
    tft.setRotation(3);
    tft.fillRect(xOffset, 2, rpmWidth, rpmHeight, color);
    tft.fillRect(xOffset + rpmWidth + rpmRightMargin, 0, screenWidth - xOffset, rpmBackHeight, ILI9341_BLACK);
  }
}

void drawWarn(bool warn1, bool warn2, bool warn3) { //1 = coolant temp, 2 = brake temp, 3 = low battery voltage
   tft.setRotation(3);
  tft.fillRect(100, 100, 60, 140, warnColor); //warning area
  if (warn1)
    tft.fillTriangle(130, 115, 115, 140, 145, 140, warnLightColor);
  if (warn2)
    tft.fillCircle(130, 170, 15, warnLightColor);
  if (warn3)
    tft.fillRect(115, 195, 30, 30, warnLightColor);
}
