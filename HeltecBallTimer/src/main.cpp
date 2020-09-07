// Example from here: https://robotzero.one/heltec-wifi-kit-32/
#include <Arduino.h>
#include <U8x8lib.h>

// the OLED used
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

#define DEBUG 0

// These define which pins on the ESP32 are used for reading the sensor values
// Interrupts are used.
// To support each of the 5 measurements, we need 6 pins, start pins and 5 others
#define START_PIN 13
#define SENSOR1_PIN 12
#define SENSOR2_PIN 14
#define SENSOR3_PIN 27
#define SENSOR4_PIN 26
#define SENSOR5_PIN 25

#define RESET_PIN 18

#define NUM_SENSORS 6

#define MAX_STR_LEN 32

// Current Position of ball
int ballPosition = -1;

// Times for each sensor
long timers[NUM_SENSORS - 1];

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
 * Update the display with the current running time.
 */
void updateDisplayTimes() {
  char timeString[MAX_STR_LEN];
  long elapsedTime = millis() - timer;

#ifdef DEBUG
  Serial.print("Running time");
  Serial.println(timeString);
#endif

  for (int i = 0; i < ballPosition; i++) {
    snprintf(timeString,MAX_STR_LEN,"%'ldmS",timers[i]);
    u8x8.drawString(1,3+i,timeString);
  }

  snprintf(timeString,MAX_STR_LEN,"%'ldmS",elapsedTime);

  for (int i = ballPosition; i < NUM_SENSORS - 1; i++) {
    u8x8.drawString(1,3+i,timeString);
  }
}

/**
 * Setup method is called once at the start.
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
  pinMode(START_PIN, INPUT_PULLUP);
  pinMode(SENSOR1_PIN, INPUT_PULLUP);
  pinMode(SENSOR2_PIN, INPUT_PULLUP);
  pinMode(SENSOR3_PIN, INPUT_PULLUP);
  pinMode(SENSOR4_PIN, INPUT_PULLUP);
  pinMode(SENSOR5_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(START_PIN),startTimer,FALLING);

  attachInterrupt(digitalPinToInterrupt(SENSOR1_PIN),takeTime1,FALLING);
  attachInterrupt(digitalPinToInterrupt(SENSOR2_PIN),takeTime2,FALLING);
  attachInterrupt(digitalPinToInterrupt(SENSOR3_PIN),takeTime3,FALLING);
  attachInterrupt(digitalPinToInterrupt(SENSOR4_PIN),takeTime4,FALLING);
  attachInterrupt(digitalPinToInterrupt(SENSOR5_PIN),takeTime5,FALLING);

  pinMode(RESET_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RESET_PIN),resetTimer,FALLING);

  memset(timers,0,sizeof(timers));

  // Draw the static text on the display
  u8x8.drawString(0,0,"Ball Timer");
  u8x8.drawString(0,1,"----------");
  displayTimer = millis();
}

// Main loop. Called repeatedly
void loop()
{
  // Start the timer running
  if (start && timerRunningFlag == false && finishTriggered == 0) {
    u8x8.clearLine(3);
    u8x8.clearLine(4);
    u8x8.clearLine(5);
    u8x8.clearLine(6);
    u8x8.clearLine(7);

    timerRunningFlag = true;
    timer = millis();
  }

  if (timerRunningFlag == true) {
    // Only update the display every 200mS
    if (millis() - displayTimer > 200) {
      displayTimer = millis();

      updateDisplayTimes();
      
#if DEBUG
      Serial.print("Ball Position: ");
      Serial.println(ballPosition);
#endif
    }
  }

  if (finish && timerRunningFlag == true) {
    timerRunningFlag = false;
    finish = false;

    updateDisplayTimes();

#if DEBUG
    Serial.println("Finished : ");
    for (int i = 0; i < NUM_SENSORS - 1; i++) {
      Serial.print(timers[i]);
      Serial.println("mS");
    }
#endif
  }

  if (resetFlag == true) {
    #if DEBUG
    Serial.println("Reset occurred");
    #endif
    startTriggered = 0;
    start = false;
    finishTriggered = 0;
    finish = false;

    ballPosition = -1;

    timerRunningFlag = false;

    memset(timers,0,sizeof(timers));
    
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
 * Interrupt handler for sensor 1
 */
void IRAM_ATTR takeTime1() {
  detachInterrupt(SENSOR1_PIN);
  ballPosition = 1;
  timers[0] = millis() - timer;
}

/**
 * Interrupt handler for sensor 2
 */
void IRAM_ATTR takeTime2() {
  detachInterrupt(SENSOR2_PIN);
  ballPosition = 2;
  timers[1] = millis() - timer;
}

/**
 * Interrupt handler for sensor 3
 */
void IRAM_ATTR takeTime3() {
  detachInterrupt(SENSOR3_PIN);
  ballPosition = 3;
  timers[2] = millis() - timer;
}

/**
 * Interrupt handler for sensor 4
 */
void IRAM_ATTR takeTime4() {
  detachInterrupt(SENSOR4_PIN);
  ballPosition = 4;
  timers[3] = millis() - timer;
}

/**
 * Interrupt handler for sensro 5, this is currently the last sensor
 */
void IRAM_ATTR takeTime5() {
  detachInterrupt(SENSOR5_PIN);
  ballPosition = 5;
  timers[4] = millis() - timer;
  finish = true;
}

/**
 * Interrupt handler for starting the timer when the ball passes it
 */
void IRAM_ATTR startTimer() {
  // Detach the start interrupt so we ignore the extra messages
  // The ball passing the sensor can generate > 100 interrupts
  detachInterrupt(START_PIN);

  ballPosition = 0;
  startTriggered++;
  start = true;
}

/**
 * Interrupt handler for resetting the timer
 */
void IRAM_ATTR resetTimer() {
  resetFlag = true;
}