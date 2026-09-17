#include <kaulab.h>

// Line sensor states:
// 3 = no line
// 0 = both on line
// 2 = right on line
// 1 = left on line

// Motor identifiers: 1 = Right motor, 2 = Left motor
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
// TICK & TIMING CONSTRAINTS (1 Tick = 16 ms)
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

// ==================================================
// LED BLINKER
// ==================================================
void updateBlinker() {  
    blinkCounter++;
    if (blinkCounter >= 10) {
        blinkCounter = 0;
        blinkState = !blinkState;
    }
    
    if (turningRight) {
        for (int i = 0; i < 12; i++) {
            zLedRing.setColorAt(i, (blinkState && (i == 4 || i == 5 || i == 6)) ? 0 : 0, 
                                   (blinkState && (i == 4 || i == 5 || i == 6)) ? 255 : 0, 0);
        }
    } else if (turningLeft) {
        for (int i = 0; i < 12; i++) {
            zLedRing.setColorAt(i, (blinkState && (i == 10 || i == 11 || i == 0)) ? 0 : 0, 
                                   (blinkState && (i == 10 || i == 11 || i == 0)) ? 255 : 0, 0);
        }
    } else {
        for (int i = 0; i < 12; i++) {
            zLedRing.setColorAt(i, statusR, statusG, statusB);
        }
    }
    zLedRing.show();
}

// ==================================================
// LINE FOLLOWING MOVEMENTS
// ==================================================
void goRight() {
    turningRight = true; turningLeft = false;
    statusR = 0; statusG = 255; statusB = 0;
    zRobotSetMotorSpeed(1, -(rightMotor));
    zRobotSetMotorSpeed(2, leftMotor);
}

void goLeft() {
    turningRight = false; turningLeft = true;
    statusR = 0; statusG = 255; statusB = 0;
    zRobotSetMotorSpeed(1, rightMotor);
    zRobotSetMotorSpeed(2, -leftMotor);
}

void findLine() {
    turningRight = false; turningLeft = false;
    statusR = 255; statusG = 255; statusB = 0; // Yellow for searching
    zRobotSetMotorSpeed(1, rightMotor + 10);
    zRobotSetMotorSpeed(2, -leftMotor + 20);   
}

void goStraight() {
    turningRight = false; turningLeft = false;
    statusR = 0; statusG = 0; statusB = 0;
    zRobotSetMotorSpeed(1, rightMotor);
    zRobotSetMotorSpeed(2, leftMotor);
}

void stopRobot() {
    turningRight = false; turningLeft = false;
    zRobotSetMotorSpeed(1, 0);
    zRobotSetMotorSpeed(2, 0);
}

// ==================================================
// FOLLOW LINE
// ==================================================
void followLine() {
    int lineread = zRobotGetLineSensor();
    
    if (lineread == 1) goLeft();
    else if (lineread == 2) goRight();
    else if (lineread == 0) goStraight();
    else if (lineread == 3) findLine();
}

// ==================================================
// OBSTACLE AVOIDANCE (TICK-CONSTRAINED STATE MACHINE)
// ==================================================
void avoidObstacle() {
    // HARD CONSTRAINT: Abort if we exceed the 5-second (312 tick) limit
    if (avoidTickCount >= MAX_AVOID_TICKS) {
        Serial.println("AVOIDANCE TIMEOUT: Max ticks reached. Stopping.");
        statusR = 255; statusG = 0; statusB = 0; // Solid Red for error
        stopRobot();
        avoiding = false; 
        return;
    }

    avoidTickCount++;
    stepTickCount++;

    // SMART RULE: If we are in the final search phases (step 3 or 4) and we see the line, 
    // abort the rest of the avoidance sequence immediately to save ticks.
    if (avoidStep >= 3 && zRobotGetLineSensor() != 3) {
        Serial.println("Line re-acquired! Resuming normal follow.");
        avoiding = false;
        avoidStep = 0;
        avoidTickCount = 0;
        stepTickCount = 0;
        goStraight();
        return;
    }

    // TIMING RULE: Only advance to the next sub-step after STEP_TICKS (800ms) have passed.
    // This prevents the robot from twitching through all states instantly.
    if (stepTickCount >= STEP_TICKS) {
        avoidStep++;
        stepTickCount = 0; // Reset counter for the new step
        Serial.print("Avoidance Step Advanced to: ");
        Serial.println(avoidStep);
    }

    // Execute the movement for the current step
    switch (avoidStep) {
        case 0: goRight(); break;      // 1. Turn right to clear the obstacle
        case 1: goStraight(); break;   // 2. Drive straight past the obstacle
        case 2: goLeft(); break;       // 3. Turn left to head back toward the original path
        case 3: goStraight(); break;   // 4. Drive straight to intersect the line
        case 4: findLine(); break;     // 5. If still not on line, spin/scan to find it
        default: findLine(); break;    // Fallback: keep searching until line is found or timeout
    }
}

// ==================================================
// MAIN OBSTACLE CHECKER
// ==================================================
void obstacle() {
    int distance = zRobotGetUltraSensor();

    // PERSISTENCE RULE: If we are already avoiding, continue the state machine 
    // regardless of the current ultrasonic reading (which changes as the robot turns).
    if (avoiding) {
        avoidObstacle();
        return;
    }

    // If not avoiding, check for a new obstacle
    if (distance > 0 && distance < 10) {
        Serial.println("OBSTACLE DETECTED - INITIATING AVOIDANCE");
        statusR = 255; statusG = 0; statusB = 0;
        
        // Initialize avoidance constraints
        avoiding = true;
        avoidStep = 0;
        avoidTickCount = 0;
        stepTickCount = 0;
        
        avoidObstacle(); // Execute step 0 immediately
    } else {
        // No obstacle, or avoidance successfully finished
        followLine();
    }
}

// ==================================================
// SETUP & LOOP
// ==================================================
void setup() {
    zInitialize();
    Serial.begin(9600);
    
    // IMPORTANT: Ensure this interval matches your "1 tick = 16ms" definition.
    // If the 2nd parameter is in ticks, '1' means 16ms. If it's in ms, use '16'.
    // Adjust based on your specific Kaulab framework documentation.
    zScheduleTask(obstacle, 1, 1);      
    zScheduleTask(updateBlinker, 3, 2); // Blinker updates every ~48ms

    zStart();
}

void loop() {
    // zStart() handles the scheduled tasks in the background. Leave empty.
    // Or, if your framework requires it, call zRun() or similar here.
}
