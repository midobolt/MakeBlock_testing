#include <kaulab.h>
// void obstacle_Calc(){
//   int s = 0;
//   int t = 0;
//   int v = 0;
//   if ()
// }
// 3 = no line
// 0 = both on line
// 2 = right on line
// 1 = left on line

// 1 = Right motor
// 2 = left motor
bool turningRight = false;
bool turningLeft = false;

int rightMotor = -150;
int leftMotor = 150;

bool blinkState = false;
int blinkCounter = 0;

int statusR = 0;
int statusG = 0;
int statusB = 0;

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


void goRight(){
    turningRight = true;
    turningLeft = false;
      statusR = 0;
      statusG = 255;
      statusB = 0;
    zRobotSetMotorSpeed(1, rightMotor);
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
  zRobotSetMotorSpeed(1, rightMotor);
    zRobotSetMotorSpeed(2, -leftMotor);   } 
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
    Serial.println("Sensor 2 + 1 is on line going STRAIGHT");
  } else if (lineread == 3){
     findLine();
    Serial.println("Finding line");

  }
}
void obstacle(){
 int distance = zRobotGetUltraSensor();

 if(distance > 0 && distance < 25){
      statusR = 255;
      statusG = 0;
      statusB = 0;
   zRobotSetMotorSpeed(1, -70);
   zRobotSetMotorSpeed(2, -50);
 } else {
    // No object detected
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
