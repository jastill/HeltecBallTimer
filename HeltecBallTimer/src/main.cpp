// Example from here: https://robotzero.one/heltec-wifi-kit-32/
#include <Arduino.h>
#include <U8x8lib.h>

// the OLED used
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

#define DEBUG 0

// These define which pins on the ESP32 are used for reading the sensor values
// Interrupts are used.
// To support each of the 5 measurements, we need 6 pins, start pins and 5 others
#define START_PIN 0
#define SENSOR1_PIN 5
#define SENSOR2_PIN 18
#define SENSOR3_PIN 23
#define SENSOR4_PIN 19
#define SENSOR5_PIN 22

#define RESET_PIN 18

#define MAX_STR_LEN 32

// Current Position of ball
unsigned int ballPosition = 0;

// Times for each sensor
long timer1 = 0;
long timer2 = 0;
long timer3 = 0;
long timer4 = 0;
long timer5 = 0;

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
void IRAM_ATTR resetTimer();
void IRAM_ATTR takeTime1();
void IRAM_ATTR takeTime2();
void IRAM_ATTR takeTime3();
void IRAM_ATTR takeTime4();
void IRAM_ATTR takeTime5();

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
  attachInterrupt(digitalPinToInterrupt(START_PIN),startTimer,FALLING);

  attachInterrupt(digitalPinToInterrupt(SENSOR1_PIN),takeTime1,FALLING);
  attachInterrupt(digitalPinToInterrupt(SENSOR2_PIN),takeTime2,FALLING);
  attachInterrupt(digitalPinToInterrupt(SENSOR3_PIN),takeTime3,FALLING);
  attachInterrupt(digitalPinToInterrupt(SENSOR4_PIN),takeTime4,FALLING);
  attachInterrupt(digitalPinToInterrupt(SENSOR5_PIN),takeTime5,FALLING);

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

  // Start the timer running
  if (start && timerRunningFlag == false && finishTriggered == 0) {
    u8x8.clearLine(3);
    u8x8.clearLine(4);
    u8x8.clearLine(5);
    u8x8.clearLine(6);
    u8x8.clearLine(7);

    timerRunningFlag = true;
    timer = millis();

    // Detach the start interrupt so we ignore the extra messages
    detachInterrupt(START_PIN);
  }

  switch (ballPosition) {
    case 1:
    detachInterrupt(SENSOR1_PIN);
    break;

    case 2:
    detachInterrupt(SENSOR2_PIN);
    break;

    case 3:
    detachInterrupt(SENSOR3_PIN);
    break;

    case 4:
    detachInterrupt(SENSOR4_PIN);
    break;

    case 5:
    detachInterrupt(SENSOR5_PIN);
    break;
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
    char timeString[MAX_STR_LEN];
    snprintf(timeString,32,"%ldmS",timer1);

    for (int i = ballPosition; i < 5; i++) {
      u8x8.drawString(1,3+i,timeString);
    }
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
    
    u8x8.clearLine(3);
    u8x8.clearLine(4);
    u8x8.clearLine(5);
    u8x8.clearLine(6);
    u8x8.clearLine(7);

    // Re attach the start interrupt
    attachInterrupt(digitalPinToInterrupt(START_PIN),startTimer,FALLING);
    attachInterrupt(digitalPinToInterrupt(SENSOR1_PIN),takeTime1,FALLING);
    attachInterrupt(digitalPinToInterrupt(SENSOR2_PIN),takeTime2,FALLING);
    attachInterrupt(digitalPinToInterrupt(SENSOR3_PIN),takeTime3,FALLING);
    attachInterrupt(digitalPinToInterrupt(SENSOR4_PIN),takeTime4,FALLING);
    attachInterrupt(digitalPinToInterrupt(SENSOR5_PIN),takeTime5,FALLING);
    resetFlag = false;
  }
}

/**
 * Interrupt handler for stopping the timer when the ball passes it
 */
void IRAM_ATTR takeTime1() {
  ballPosition = 1;
  timer1 = millis() - timer;
}

/**
 * Interrupt handler for stopping the timer when the ball passes it
 */
void IRAM_ATTR takeTime2() {
  ballPosition = 2;
  timer2 = millis() - timer;
}

/**
 * Interrupt handler for stopping the timer when the ball passes it
 */
void IRAM_ATTR takeTime3() {
  ballPosition = 3;
  timer3 = millis() - timer;
}

/**
 * Interrupt handler for stopping the timer when the ball passes it
 */
void IRAM_ATTR takeTime4() {
  ballPosition = 4;
  timer4 = millis() - timer;
}

/**
 * Interrupt handler for stopping the timer when the ball passes it
 */
void IRAM_ATTR takeTime5() {
  ballPosition = 5;
  timer5 = millis() - timer;
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