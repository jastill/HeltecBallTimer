// Example from here: https://robotzero.one/heltec-wifi-kit-32/
#include <Arduino.h>
#include <U8x8lib.h>

// the OLED used
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

#define DEBUG 1
#define START_PIN 0
#define FINISH_PIN 22

long timer = 0;
long displayTimer = 0;
long startTime = 0;
int timerRunningFlag = false;

void setup()
{
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);

  pinMode(START_PIN,INPUT_PULLDOWN);
  pinMode(FINISH_PIN,INPUT_PULLDOWN);

  Serial.begin(115200);

  u8x8.drawString(0,0,"Ball Timer");
  u8x8.drawString(0,1,"----------");
  displayTimer = millis();
}

void loop()
{
  // Only update the display every 200mS
  if (millis() - displayTimer > 200) {
    displayTimer = millis();
  }

  int start = 0x01 ^ digitalRead(START_PIN);
  int finish = 0x01 ^  digitalRead(FINISH_PIN);

#if DEBUG
  char debug[32];
  sprintf(debug,"Start %i",start);
  u8x8.drawString(1,6,debug);
  sprintf(debug,"Finish %i",finish);
  u8x8.drawString(1,7,debug);

  Serial.print("Start: ");
  Serial.println(start);
  Serial.print("Finish: ");
  Serial.println(finish);
#endif

  if (start && timerRunningFlag == false) {
    timerRunningFlag = true;
    timer = millis();

    Serial.println("Started");
  }

  if (finish && timerRunningFlag == true) {
    Serial.print("Finished : ");
    Serial.print(millis() - timer);
    Serial.println("mS");

    timerRunningFlag = false;
  }

  if (timerRunningFlag == true) {
    long runningTime = millis() - timer;
    char timeString[10];
    sprintf(timeString,"%ldmS",runningTime);
    u8x8.drawString(1,4,timeString);
  }
}