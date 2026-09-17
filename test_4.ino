#include <kaulab.h>

// 3 = no line
// 0 = both on line
// 2 = right on line
// 1 = left on line

// 1 = Right motor
// 2 = left motor
bool turningRight = false;
bool turningLeft = false;

int rightMotor = -120;
int leftMotor = 120;

bool blinkState = false;
int blinkCounter = 0;

int statusR = 0;
int statusG = 0;
int statusB = 0;


// ==================================================
// LED BLINKER
// ==================================================
void updateBlinker() {  
        blinkCounter++;
     if (blinkCounter >= 10) {
            blinkCounter = 0;
            blinkState = !blinkState;
        }
    // RIGHT TURN
    if (turningRight) {
            // Blinker ON
            for (int i = 0; i < 12; i++) {
                if ( blinkState && (i == 4 || i == 5 || i == 6)) {
                    zLedRing.setColorAt(i, 0, 255, 0);
                } 
                else {
                    zLedRing.setColorAt(i, 0, 0, 0);
                }
            }
        } 
        else if (turningLeft){
          for (int i = 0; i < 12; i++) {
            if (blinkState &&
                (i == 10 || i == 11 || i == 0)) {

                zLedRing.setColorAt(i, 0, 255, 0);
            }
            else {
                zLedRing.setColorAt(i, 0, 0, 0);
            }
          }
        } else  {

            // Blinker OFF
            for (int i = 0; i < 12; i++) {
                zLedRing.setColorAt(i, statusR, statusG, statusB);
            }
        }
        zLedRing.show();
}

// ==================================================
// LINE FOLLOWING MOVEMENTS
// ==================================================

void goRight(){
    turningRight = true;
    turningLeft = false;
      statusR = 0;
      statusG = 255;
      statusB = 0;
    zRobotSetMotorSpeed(1, -(rightMotor));
    zRobotSetMotorSpeed(2, leftMotor);
}
void goLeft(){
      turningRight = false;
      turningLeft = true;
      statusR = 0;
      statusG = 255;
      statusB = 0;
    zRobotSetMotorSpeed(1, rightMotor);
    zRobotSetMotorSpeed(2, -leftMotor);
}
void findLine(){
        turningRight = false;
        turningLeft = false;
      statusR = 255;
      statusG = 255;
      statusB = 0;
  zRobotSetMotorSpeed(1, rightMotor + 10);
    zRobotSetMotorSpeed(2, -leftMotor + 20);   
}

void goStraight(){
        turningRight = false;
        turningLeft = false;
      // zSetAllLed(255, 255, 255);
      statusR = 0;
      statusG = 0;
      statusB = 0;
    zRobotSetMotorSpeed(1, rightMotor);
    zRobotSetMotorSpeed(2, leftMotor);
}


// ==================================================
// FOLLOW LINE
// ==================================================

void followLine(){
  int lineread = zRobotGetLineSensor();
    
  if (lineread == 1){
    goLeft();
    Serial.println("Sensor 2 is on line going RIGHT");

  } else if(lineread == 2){
    goRight();
    Serial.println("Sensor 1 is on line going LEFT");

  } else if(lineread == 0){
    goStraight();
    Serial.println("Sensor 1 + 2 is on line going STRAIGHT");

  } else if (lineread == 3){
     findLine();

    Serial.println("Finding line");
  }
}

// ==================================================
// OBSTACLE AVOIDANCE
// ==================================================

// ==================================================
const int MS_PER_TICK = 16;

// RULE 1: How long should each sub-movement last? 
// 800 ms / 16 ms = 50 ticks. 
// This gives the robot ~0.8 seconds to complete a turn or short straight movement.
const int STEP_DURATION_MS = 800;
const int STEP_TICKS = STEP_DURATION_MS / MS_PER_TICK; 

// RULE 2: What is the absolute maximum time allowed for the whole avoidance maneuver?
// 5000 ms / 16 ms = 312 ticks (Exactly 5 seconds).
// If the robot hasn't found the line after 5 seconds, it aborts to prevent infinite loops/battery drain.
const int MAX_AVOID_DURATION_MS = 5000;
const int MAX_AVOID_TICKS = MAX_AVOID_DURATION_MS / MS_PER_TICK;

bool avoiding = false;
int avoidStep = 0;
int avoidTickCount = 0;      // Total ticks spent in the entire avoidance maneuver
int stepTickCount = 0;       // Ticks spent in the current sub-step
 


void avoidObstacle() {
  avoidTickCount++;
  stepTickCount++;

      if (avoidStep >= 3 && zRobotGetLineSensor() != 3) {
        Serial.println("Line re-acquired! Resuming normal follow.");
        avoiding = false;
        avoidStep = 0;
        avoidTickCount = 0;
        stepTickCount = 0;
        goStraight();
        return;
    }

    if (stepTickCount >= STEP_TICKS) {
        avoidStep++;
        stepTickCount = 0; 
        Serial.print("Avoidance Step Advanced to: ");
        Serial.println(avoidStep);
    }

  switch (avoidStep) {

        case 0:
            goRight();
            break;
        case 1:
            goStraight();
            break;
        case 2:
            goLeft();
            break;
        case 3:
            goStraight();
            break;
        case 4: 
             findLine(); 
             break;   
        default: 
            findLine(); 
            break; 
    }
}
    

void obstacle() {

  int distance = zRobotGetUltraSensor();
 if (avoiding) {
        avoidObstacle();
        return;
    }
    
  if( distance > 0 && distance < 10){
      statusR = 255;
      statusG = 0;
      statusB = 0;
    Serial.println("OBSTACLE");
    avoiding = true;
    avoidStep = 0;
    avoidTickCount = 0;
    stepTickCount = 0;
    
    avoidObstacle();
      
  } else {
    followLine();
  }
}

void setup() {
  // put your setup code here, to run once:
  zInitialize();
  Serial.begin(9600);
  zScheduleTask(obstacle, 3, 1);
 zScheduleTask(updateBlinker, 3, 3);

  zStart();
}

void loop() {
}
