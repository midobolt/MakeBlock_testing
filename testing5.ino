#include <kaulab.h>

// 3 = no line
// 0 = both on line
// 2 = right on line
// 1 = left on line

// 1 = Right motor
// 2 = left motor

int statusR = 0;
int statusG = 0;
int statusB = 0;

int rightMotor = -100;
int leftMotor = 100;

boolean turningRight = false;
boolean turningLeft = false;

int avoidState = 0;          // 0=Normal, 1=Backing Up, 2=Turning Right, 3=Finding Line
unsigned long avoidStartTime = 0; // Tracks time for non-blocking delays


// ==================================================
// 1. UNIFIED SENSOR & CONTROL LOOP
// ==================================================
//void updateRobotState() {
//  int distance = zRobotGetUltraSensor();
//  int sensor = zRobotGetLineSensor();
//
//  if (distance >= 0 && distance < 10) {
//    handleObstacle(distance, sensor);
//  }
//  else {
//    handleLineFollowing(sensor);
//  }
//}

void updateRobotState() {
  int distance = zRobotGetUltraSensor();
  int sensor = zRobotGetLineSensor();

  
  if (avoidState > 0) {
    handleObstacle(distance, sensor);
  } 
  else {
    if (distance >= 0 && distance < 10) {
      handleObstacle(distance, sensor); 
    } else {
      handleLineFollowing(sensor);
    }
  }
}


// ==================================================
// 2. BEHAVIOR DECISIONS
// ==================================================
//void handleObstacle(int distance, int sensor) {
//  Serial.println("Obstacle detected! Avoiding.");
//
//  if (distance >= 0 && distance < 10) {
//    goBack();
//  } else if (distance < 13) {
//    goRight();
//  } else if (sensor == 3) {
//    findLine();
//  }
//}
void handleObstacle(int distance, int sensor) {
  // Trigger the sequence if we are in normal state and hit an obstacle
  if (avoidState == 0) {
    Serial.println("Obstacle detected! Starting avoidance sequence.");
    avoidState = 1;
    avoidStartTime = millis(); // Start the timer
  }

  // STEP 1: Back up for ~600ms (Adjust this number if 3cm takes more/less time)
  if (avoidState == 1) {
    goBack();
    if (millis() - avoidStartTime >= 600) { 
      avoidState = 2;
      avoidStartTime = millis(); // Reset timer for the next step
      Serial.println("Backed up. Now turning right to go around.");
    }
  } 
  
  // STEP 2: Turn right until the ultrasonic sensor sees open space (> 15 cm)
  else if (avoidState == 2) {
    goRight();
    int currentDist = zRobotGetUltraSensor(); // Read fresh distance
    // 15cm is a safe buffer. -1 is often returned by sensors when no obstacle is detected
    if (currentDist > 15 || currentDist == -1) { 
      avoidState = 3;
      Serial.println("Path clear. Searching for line.");
    }
  } 
  
 
  else if (avoidState == 3) {
    findLine();
    if (sensor == 0 || sensor == 3) {
      avoidState = 0; 
      Serial.println("Line found! Resuming normal operation.");
    }
  }
}
void handleLineFollowing(int sensor) {
  if (sensor == 1) {
    goLeft();
  } else if (sensor == 2) {
    goRight();
  } else if (sensor == 0) {
    goStraight();
  } else if (sensor == 3) {
    findLine();
  }
}

// ==================================================
// 3. MOVEMENT FUNCTIONS
// ==================================================
void goRight() {
  turningRight = true;
  turningLeft = false;
  statusR = 0; statusG = 255; statusB = 0;
  zRobotSetMotorSpeed(1, -(rightMotor) - 60);
  zRobotSetMotorSpeed(2, leftMotor);
  Serial.println("Going Right");

}

void goLeft() {
  turningRight = false;
  turningLeft = true;
  statusR = 0; statusG = 255; statusB = 0;
  zRobotSetMotorSpeed(1, rightMotor);
  zRobotSetMotorSpeed(2, -leftMotor + 60);
  Serial.println("Going Left");

}

void findLine() {
  turningRight = false;
  turningLeft = false;
  statusR = 255; statusG = 255; statusB = 0;
  zRobotSetMotorSpeed(1, rightMotor + 20);
  zRobotSetMotorSpeed(2, -leftMotor + 30);
  Serial.println("Finding Line");

}

void goStraight() {
  turningRight = false;
  turningLeft = false;
  statusR = 0; statusG = 0; statusB = 0;
  zRobotSetMotorSpeed(1, rightMotor);
  zRobotSetMotorSpeed(2, leftMotor);
  Serial.println("Going straight");
}
void goBack() {
  zRobotSetMotorSpeed(1, -rightMotor);
  zRobotSetMotorSpeed(2, -leftMotor);
  Serial.println("Going Back");

}

// ==================================================
// 4. SETUP & LOOP
// ==================================================
void setup() {
  zInitialize();
  Serial.begin(9600);

  zScheduleTask(updateRobotState, 50, 40);

  zStart();
}

void loop() {

}
