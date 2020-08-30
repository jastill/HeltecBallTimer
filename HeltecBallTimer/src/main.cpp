// Example from here: https://robotzero.one/heltec-wifi-kit-32/
#include <Arduino.h>
#include <U8x8lib.h>

// the OLED used
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

#define DEBUG 1
#define START_PIN 0
#define FINISH_PIN 22

// Timer for ball drop
long timer = 0;

// Timer for display updates
long displayTimer = 0;

// Time ball passes first sensor
long startTime = 0;

// Flag to show timer running
int timerRunningFlag = false;

/**
 * Setup method is called once at the start.
 * 
 */
void setup()
{
  #if DEBUG
  Serial.begin(115200);
  #endif

  // Enable the display
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);

  // Setup the sensors
  pinMode(START_PIN,INPUT_PULLDOWN);
  pinMode(FINISH_PIN,INPUT_PULLDOWN);

  // Draw the static text on the display
  u8x8.drawString(0,0,"Ball Timer");
  u8x8.drawString(0,1,"----------");
  displayTimer = millis();
}

// Main loop. Called repeatedly
void loop()
{
  // Read the sensor values
  int start = 0x01 ^ digitalRead(START_PIN);
  int finish = 0x01 ^  digitalRead(FINISH_PIN);

  // Only update the display every 200mS
  if (millis() - displayTimer > 200) {
    displayTimer = millis();
  }

  // Display the current sensor values
  char debug[32];
  sprintf(debug,"Start %i",start);
  u8x8.drawString(1,6,debug);
  sprintf(debug,"Finish %i",finish);
  u8x8.drawString(1,7,debug);

#if DEBUG
    Serial.print("Start: ");
    Serial.println(start);
    Serial.print("Finish: ");
    Serial.println(finish);
#endif

  // Start the timer running
  if (start && timerRunningFlag == false) {
    timerRunningFlag = true;
    timer = millis();

#if DEBUG
    Serial.println("Started");
#endif
  }

  //
  if (finish && timerRunningFlag == true) {
#if DEBUG
    Serial.print("Finished : ");
    Serial.print(millis() - timer);
    Serial.println("mS");
#endif

    timerRunningFlag = false;
  }

  if (timerRunningFlag == true) {
    long runningTime = millis() - timer;
    char timeString[10];
    sprintf(timeString,"%ldmS",runningTime);
    u8x8.drawString(1,4,timeString);
  }
}