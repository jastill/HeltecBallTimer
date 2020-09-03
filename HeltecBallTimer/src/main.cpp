// Example from here: https://robotzero.one/heltec-wifi-kit-32/
#include <Arduino.h>
#include <U8x8lib.h>

// the OLED used
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

#define DEBUG 0
#define START_PIN 0
#define FINISH_PIN 22

#define RESET_PIN 18

#define MAX_STR_LEN 32

// Timer for ball drop
long timer = 0;

// Timer for display updates
long displayTimer = 0;

// Time ball passes first sensor
long startTime = 0;

// Flag to show timer running
int timerRunningFlag = false;

// Interrupt flags
int finish = false;
int finishTriggered = 0;
int start = false;
int startTriggered = 0;
int resetFlag = false;

// Forward declaration
void IRAM_ATTR startTimer();
void IRAM_ATTR stopTimer();
void IRAM_ATTR resetTimer();

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
  attachInterrupt(digitalPinToInterrupt(FINISH_PIN),stopTimer,FALLING);
  attachInterrupt(digitalPinToInterrupt(START_PIN),startTimer,FALLING);

  pinMode(RESET_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RESET_PIN),resetTimer,FALLING);

  // Draw the static text on the display
  u8x8.drawString(0,0,"Ball Timer");
  u8x8.drawString(0,1,"----------");
  displayTimer = millis();
}

// Main loop. Called repeatedly
void loop()
{
  // Only update the display every 200mS
  if (millis() - displayTimer > 200) {
    displayTimer = millis();
  }

  // Display the current sensor values
  // This uses an array of characters
  char debug[32];
  sprintf(debug,"Start %i %i",start,startTriggered);
  u8x8.drawString(1,6,debug);
  sprintf(debug,"Finish %i %i",finish,finishTriggered);
  u8x8.drawString(1,7,debug);

  // Start the timer running
  if (start && timerRunningFlag == false && finishTriggered == 0) {
    u8x8.clearLine(4);
    timerRunningFlag = true;
    timer = millis();

    // Detach the start interrupt so we ignore the extra messages
    detachInterrupt(START_PIN);

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
    finish = false;
  }

  if (timerRunningFlag == true) {
    long runningTime = millis() - timer;
    char timeString[MAX_STR_LEN];
    snprintf(timeString,32,"%ldmS",runningTime);
    u8x8.drawString(1,4,timeString);
  }

  if (resetFlag == true) {
    #if DEBUG
    Serial.println("Reset occurred");
    #endif
    startTriggered = 0;
    start = false;
    finishTriggered = 0;
    finish = false;

    timerRunningFlag = false;
    
    u8x8.clearLine(4);
    u8x8.clearLine(6);
    u8x8.clearLine(7);
    u8x8.drawString71,4,"0mS");

    // Re attach the start interrupt
    attachInterrupt(digitalPinToInterrupt(START_PIN),startTimer,FALLING);
    resetFlag = false;
  }
}

/**
 * Interrupt handler for stopping the timer when the ball passes it
 */
void IRAM_ATTR stopTimer() {
  finishTriggered++;
  finish = true;
}

/**
 * Interrupt handler for starting the timer when the ball passes it
 */
void IRAM_ATTR startTimer() {
  startTriggered++;
  start = true;
}

/**
 * Interrupt handler for resetting the timer
 */
void IRAM_ATTR resetTimer() {
  resetFlag = true;
}