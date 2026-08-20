#include <Wire.h>
#include "Adafruit_AS726x.h"
#include <Servo.h>

// -------- AS7262 (LINE SENSOR) --------
Adafruit_AS726x ams;
#define LED_AS 30
uint16_t sensorValues[AS726x_NUM_CHANNELS];

// -------- TCS3200 (BLOCK/CLAW SENSOR) --------
#define S0 19
#define S1 18
#define S2 36
#define S3 34
#define sensorOut 32

int redFrequency = 0;
int greenFrequency = 0;
int blueFrequency = 0;

// -------- MOTORS --------
#define M1A 4
#define M1B 5
#define M2A 6
#define M2B 7

int forwardLeft = 60;
int forwardRight = -105;
int reverseLeft = -90;
int reverseRight = 100;

// -------- STEPPER --------
#define STEP 31
#define DIR 3

// -------- SERVO --------
#define SERVO_PIN 8
Servo myServo;

// -------- VARIABLES --------
int blockCount = 0;
int currentColor = -1;  // -1 = no block held
bool hasBlock = false;
bool isDriving = false;
bool goingForward = false;

// -------- STATES --------
enum RobotState {
  WAIT_START,
  DRIVING,
  STOPPED
};

RobotState state = WAIT_START;

//////////////////////////////////////////////////
// -------- SETUP --------
//////////////////////////////////////////////////

void setup() {
  Serial.begin(9600);

  // Motors
  pinMode(M1A, OUTPUT);
  pinMode(M1B, OUTPUT);
  pinMode(M2A, OUTPUT);
  pinMode(M2B, OUTPUT);

  // Stepper
  pinMode(STEP, OUTPUT);
  pinMode(DIR, OUTPUT);

  // Servo
  myServo.attach(SERVO_PIN);
  myServo.write(90);

  // TCS3200
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  // AS7262
  pinMode(LED_AS, OUTPUT);

  if (!ams.begin()) {
    Serial.println("AS7262 ERROR");
  } else {
    Serial.println("AS7262 OK");
  }
}

//////////////////////////////////////////////////
// -------- MAIN LOOP --------
//////////////////////////////////////////////////

void loop() {
  switch (state) {

    case WAIT_START:
      if (getGroundColor() == 3) {
        isDriving = true;
        state = DRIVING;
      }
      break;

    case DRIVING: {
      drive();
      int groundColor = getGroundColor();

      // Yellow — already driving, ignore
      if (groundColor == 3) {
        break;
      }

      // Orange — pick up if no block, pass over if carrying
      if (groundColor == 4) {
        if (!hasBlock) {
          if (blockCount == 0) {
            stopMotors();
            delay(300);
            pickUpBlock();
            delay(500);
            currentColor = getBlockColor();
            hasBlock = true;
            goingForward = !goingForward;
            driveOff();
          }
          else if (blockCount == 1) {
            drive();
            delay(300);
            pickUpBlock();
            delay(500);
            currentColor = getBlockColor();
            hasBlock = true;
            goingForward = !goingForward;
            driveOff();
          }
          else if (blockCount == 2) {
            drive();
            delay(600);
            stopMotors();
            delay(300);
            pickUpBlock();
            delay(500);
            currentColor = getBlockColor();
            hasBlock = true;
            goingForward = !goingForward;
            driveOff();
          }         // <-- THIS WAS MISSING
          else {
            break;
          }
        }
        break;
      }

      // Matching color — drop block and reverse
      if (hasBlock && groundColor == currentColor) {
        stopMotors();
        delay(300);
        dropBlock();
        delay(300);
        hasBlock = false;
        currentColor = -1;
        blockCount++;
        goingForward = !goingForward;
        if (blockCount >= 3) {
          state = STOPPED;
          break;
        }
        driveOff();
        break;
      }

      break;
    }

    case STOPPED:
      stopMotors();
      while (true);
      break;
  }
}

//////////////////////////////////////////////////
// -------- MOTOR CONTROL --------
//////////////////////////////////////////////////

void setMotor(int pinA, int pinB, int speedVal) {
  speedVal = constrain(speedVal, -255, 255);
  if (speedVal > 0) {
    analogWrite(pinA, speedVal);
    analogWrite(pinB, 0);
  } else if (speedVal < 0) {
    analogWrite(pinA, 0);
    analogWrite(pinB, -speedVal);
  } else {
    analogWrite(pinA, 0);
    analogWrite(pinB, 0);
  }
}

void setBothMotors(int leftSpeed, int rightSpeed) {
  setMotor(M1A, M1B, leftSpeed);
  setMotor(M2A, M2B, rightSpeed);
}

void drive() {
  if (goingForward) {
    setBothMotors(forwardLeft, forwardRight);
  } else {
    setBothMotors(reverseLeft, reverseRight);
  }
}

void driveOff() {
  drive();
  delay(300);
}

void stopMotors() {
  setBothMotors(0, 0);
}

//////////////////////////////////////////////////
// -------- AS7262 — GROUND COLOR --------
//////////////////////////////////////////////////

int getGroundColor() {
  ams.startMeasurement();

  bool rdy = false;
  int timeout = 0;
  while (!rdy && timeout < 1000) {
    delay(5);
    rdy = ams.dataReady();
    timeout++;
  }

  if (!rdy) {
    Serial.println("ERROR: AS7262 not responding.");
    return -1;
  }

  ams.readRawValues(sensorValues);

  int readings[6];
  readings[0] = sensorValues[0] - 700;  // Violet
  readings[1] = sensorValues[1] - 100;  // Blue
  readings[2] = sensorValues[2] - 300;  // Green
  readings[3] = sensorValues[3] - 200;  // Yellow
  readings[4] = sensorValues[4] - 800;  // Orange
  readings[5] = sensorValues[5] - 200;  // Red

  int total = 0;
  int maxIndex = 0;

  for (int i = 0; i < 6; i++) {
    total += readings[i];
    if (readings[i] > readings[maxIndex]) {
      maxIndex = i;
    }
  }

  if (total < 2000) {
    Serial.println("Ground: Black");
    return 7;
  } else if (total > 12000) {
    Serial.println("Ground: White");
    return 6;
  }

  const char* colors[6] = {"Violet", "Blue", "Green", "Yellow", "Orange", "Red"};
  Serial.print("Ground: ");
  Serial.println(colors[maxIndex]);
  return maxIndex;
}

//////////////////////////////////////////////////
// -------- TCS3200 — BLOCK COLOR --------
//////////////////////////////////////////////////

int getBlockColor() {
  delay(100);
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  redFrequency = pulseIn(sensorOut, LOW);
  delay(100);

  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);
  greenFrequency = pulseIn(sensorOut, LOW);
  delay(100);

  digitalWrite(S2, LOW);
  digitalWrite(S3, HIGH);
  blueFrequency = pulseIn(sensorOut, LOW);
  delay(100);

  Serial.print("R = "); Serial.print(redFrequency);
  Serial.print(" G = "); Serial.print(greenFrequency);
  Serial.print(" B = "); Serial.println(blueFrequency);

  if (redFrequency < greenFrequency && redFrequency < blueFrequency) {
    Serial.println("Block: Red");
    return 5;
  }
  if (greenFrequency < redFrequency && greenFrequency < blueFrequency) {
    Serial.println("Block: Green");
    return 2;
  }
  if (blueFrequency < redFrequency && blueFrequency < greenFrequency) {
    Serial.println("Block: Blue");
    return 1;
  }

  Serial.println("Block: Unknown");
  return -1;
}

//////////////////////////////////////////////////
// -------- BLOCK DETECTION --------
//////////////////////////////////////////////////

bool blockDetected() {
  return true;  // Replace with actual sensor check if available
}

//////////////////////////////////////////////////
// -------- ARM --------
//////////////////////////////////////////////////
void stepperMove(int steps, int dir) {
  digitalWrite(DIR, dir);
  for (int i = 0; i < steps; i++) {
    digitalWrite(STEP, HIGH);
    delayMicroseconds(2000);
    digitalWrite(STEP, LOW);
    delayMicroseconds(2000);
  }
}

void pickUpBlock() {
  stepperMove(1300, LOW);
  delay(300);
  myServo.write(65);
  delay(500);
  stepperMove(1300, HIGH);
}

void dropBlock() {
  myServo.write(90);
}