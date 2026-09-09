#include <kaulab.h>


int lineread;
void FollowLine(){
  lineread = zRobotGetLineSensor();

  if (lineread == 2){
    zRobotSetMotorSpeed(1, 0 );
    zRobotSetMotorSpeed(2, 150);
  } else if(lineread == 1){
    zRobotSetMotorSpeed(1, -150);
    zRobotSetMotorSpeed(2, 0 );
  } else if(lineread == 0){
    zRobotSetMotorSpeed(1, -150);
    zRobotSetMotorSpeed(2, 150);
  } else if (lineread == 3){
    zRobotSetMotorSpeed(1, -90);
    zRobotSetMotorSpeed(2, 0);    
  }
  }
//void Obstacle(){
//  distance = zRobotGetUltraSensor();
//  if(distance > 0 && distance < 20){
//    zRobotSetMotorSpeed(1, );
//    zRobotSetMotorSpeed(2, )
//  } else {
//    FollowLine();
//  }
//}
// 3 = no line
// 0 = both on line
// 2 = right on line
// 1 = left on line

// 1 = Right motor
// 2 = left motor

void setup() {
  // put your setup code here, to run once:
  zInitialize();
  zRobotSetMotorSpeed(1, -70);
  zRobotSetMotorSpeed(2, 0);

  // zScheduleTask(Obstacle(), , );
   zScheduleTask(FollowLine, 100, 70);
  
}



void loop() {
}
