#include <kaulab.h>


// 3 = no line
// 0 = both sensors on line
// 2 = right sensor on line
// 1 = left sensor on line

// 1 = right motor
// 2 = left motor

int lineSensor = 3;
int distance = 0;



int rightMotor = -120;
int leftMotor = 120;

#define TURN_TIME 25  // 25 × 16 = 400 ms
#define FORWARD_TIME 38  // 38 × 16 ≈ 608 ms
#define OBSTACLE_DISTANCE 15


enum Direction {
  CLOCKWISE,
  COUNTERCLOCKWISE
};

Direction lastDirection = CLOCKWISE;


enum RobotState {
  FOLLOW_LINE,

  AVOID_TURN_1,
  AVOID_FORWARD_1,

  AVOID_TURN_2,
  AVOID_FORWARD_2,

  AVOID_TURN_3,

  SEARCH_LINE
};
RobotState robotState = FOLLOW_LINE;
TickType_t stateStartTime = 0;



void goRight() {

  zRobotSetMotorSpeed(1,  -(rightMotor) - 70);
  zRobotSetMotorSpeed(2, leftMotor + 30);
}


void goLeft() {

  zRobotSetMotorSpeed(1, rightMotor - 30);
  zRobotSetMotorSpeed(2, -leftMotor + 70);
}


void goStraight() {

  zRobotSetMotorSpeed(1, rightMotor);
  zRobotSetMotorSpeed(2, leftMotor);
}


void findLine() {
 TickType_t now = xTaskGetTickCount();
 
      if (lastDirection == CLOCKWISE) {

        // Clockwise → turn right
        goRight();

      } else {

        // Counterclockwise → turn left
        goLeft();
      }


      if (now - stateStartTime >= TURN_TIME) {

        robotState = AVOID_FORWARD_1;

        stateStartTime = now;
      }


  zRobotSetMotorSpeed(1, rightMotor + 30);
  zRobotSetMotorSpeed(2,  -leftMotor + 40);
}


void lineFollow() {

  switch (lineSensor) {

    case 0:
      goStraight();
      break;


    case 1:
      lastDirection = COUNTERCLOCKWISE;

      goLeft();
//      zRobotSetMotorSpeed(1, -70);
//      zRobotSetMotorSpeed(2, 40);
Serial.println("Going Left");

      break;


    case 2:
      lastDirection = CLOCKWISE;
      goRight();
//      zRobotSetMotorSpeed(1, -40);
//      zRobotSetMotorSpeed(2, 70);
Serial.println("Going Right");


      break;


    case 3:
      findLine();
      break;
  }
}


void sensorTask() {

  lineSensor = zRobotGetLineSensor();

  distance = zRobotGetUltraSensor();
}

void controlTask() {

  TickType_t now = xTaskGetTickCount();


  switch (robotState) {

    case FOLLOW_LINE:

      if (distance > 0 && distance < OBSTACLE_DISTANCE) {

        Serial.println("OBSTACLE");

        robotState = AVOID_TURN_1;

        stateStartTime = now;

      } else {

        lineFollow();
      }

      break;


    case AVOID_TURN_1:

      if (lastDirection == CLOCKWISE) {

        // Clockwise → turn right
        goRight();

      } else {

        // Counterclockwise → turn left
        goLeft();
      }


      if (now - stateStartTime >= TURN_TIME) {

        robotState = AVOID_FORWARD_1;

        stateStartTime = now;
      }

      break;

    case AVOID_FORWARD_1:

      goStraight();


      if (now - stateStartTime >= FORWARD_TIME) {

        robotState = AVOID_TURN_2;

        stateStartTime = now;
      }

      break;

    case AVOID_TURN_2:

      if (lastDirection == CLOCKWISE) {

        // Opposite of first turn
        goLeft();

      } else {

        goRight();
      }


      if (now - stateStartTime >= TURN_TIME) {

        robotState = AVOID_FORWARD_2;

        stateStartTime = now;
      }

      break;

    case AVOID_FORWARD_2:

      goStraight();


      if (lineSensor != 3) {

        Serial.println("LINE FOUND");

        robotState = SEARCH_LINE;
      }

      break;

    case AVOID_TURN_3:

      if (lastDirection == CLOCKWISE) {

        // Opposite of second turn
        goRight();

      } else {

        goLeft();
      }


      if (now - stateStartTime >= TURN_TIME) {

        robotState = SEARCH_LINE;

        stateStartTime = now;
      }

      break;

    case SEARCH_LINE:

      lineFollow();

      robotState = FOLLOW_LINE;

      break;
  }
}


void setup() {

  zInitialize();

  Serial.begin(9600);

  zScheduleTask(sensorTask, 4, 1);

  zScheduleTask(controlTask, 2, 1);

  zStart();
}


void loop() {

}
