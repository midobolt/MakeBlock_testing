#include <kaulab.h>


// 3 = no line
// 0 = both sensors on line
// 2 = right sensor on line
// 1 = left sensor on line

#define OBSTACLE_DISTANCE 15

// Scheduler ticks
#define TURN_TICKS    35       // ~560 ms
#define SEARCH_TICKS  25       // ~400 ms



enum RobotState {
  LINE_FOLLOWING,
  AVOID_TURN,
  AVOID_SEARCH
};

RobotState robotState = LINE_FOLLOWING;

int stateTicks = 0;


int rightMotor = -100;
int leftMotor  = 100;

int statusR = 0;
int statusG = 0;
int statusB = 0;

boolean turningRight = false;
boolean turningLeft = false;


void handleLineFollowing(int sensor) {

  switch (sensor) {

    // Both sensors on line
    case 0:
      goStraight();
      break;

    // Left sensor on line
    case 1:
      goLeft();
      break;

    // Right sensor on line
    case 2:
      goRight();
      break;

    // No line
    case 3:
      findLine();
      break;

    default:
      goStraight();
      break;
  }
}


void startAvoidance() {

  // No reverse!
  robotState = AVOID_TURN;
  stateTicks = 0;

  Serial.println("Obstacle detected!");
  Serial.println("Turning right around obstacle.");

  turnAvoidanceRight();
}


// ==================================================
// OBSTACLE AVOIDANCE
// ==================================================

void handleObstacle() {

  switch (robotState) {

    case AVOID_TURN:

      turnAvoidanceRight();

      stateTicks++;

      if (stateTicks >= TURN_TICKS) {

        stateTicks = 0;
        robotState = AVOID_SEARCH;

        Serial.println("Turn finished.");
        Serial.println("Searching for line.");
      }

      break;
      
    case AVOID_SEARCH:

      findLine();

      stateTicks++;

      {
        int sensor = zRobotGetLineSensor();

        if (sensor != 3) {

          stateTicks = 0;
          robotState = LINE_FOLLOWING;

          Serial.println("Line found.");
          Serial.println("Resuming line following.");
        }

        // Safety timeout
        else if (stateTicks >= SEARCH_TICKS) {

          stateTicks = 0;
          robotState = LINE_FOLLOWING;

          Serial.println("Search timeout.");
          Serial.println("Resuming line following.");
        }
      }

      break;


    default:

      robotState = LINE_FOLLOWING;
      stateTicks = 0;

      break;
  }
}

void updateRobotState() {

  if (robotState != LINE_FOLLOWING) {

    handleObstacle();
    return;
  }


  // Normal line-following mode
  int distance = zRobotGetUltraSensor();

  // Obstacle detected
  if (distance > 0 && distance <= OBSTACLE_DISTANCE) {

    startAvoidance();
    return;
  }


  // No obstacle
  int sensor = zRobotGetLineSensor();

  handleLineFollowing(sensor);
}
// ==================================================
// Direction funcs
// ==================================================

void goStraight() {

  turningRight = false;
  turningLeft = false;

  statusR = 0;
  statusG = 0;
  statusB = 0;

  zRobotSetMotorSpeed(1, rightMotor);
  zRobotSetMotorSpeed(2, leftMotor);
}


void goRight() {

  turningRight = true;
  turningLeft = false;

  statusR = 0;
  statusG = 255;
  statusB = 0;

  zRobotSetMotorSpeed(1, -(rightMotor) - 60);
  zRobotSetMotorSpeed(2, leftMotor);
}


void goLeft() {

  turningRight = false;
  turningLeft = true;

  statusR = 0;
  statusG = 255;
  statusB = 0;

  zRobotSetMotorSpeed(1, rightMotor);
  zRobotSetMotorSpeed(2, -leftMotor + 60);
}


void findLine() {

  turningRight = false;
  turningLeft = false;

  statusR = 255;
  statusG = 255;
  statusB = 0;

  zRobotSetMotorSpeed(1, rightMotor + 20);
  zRobotSetMotorSpeed(2, -leftMotor + 30);
}


void setup() {

  zInitialize();

  Serial.begin(9600);

  // Same scheduler structure
  zScheduleTask(updateRobotState, 5, 3);

  zStart();
}


// ==================================================
// LOOP
// ==================================================

void loop() {

}
