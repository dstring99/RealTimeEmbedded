#ifndef ROBOT_H
#define ROBOT_H

//Constants
#define	ROBOT_STK_SIZE     256 
#define NUM_ROBOTS 13

//Type definition of Robot Status Struct
typedef struct
{
  CPU_INT08U address; // Robot address 3 <= address <= 15
  CPU_BOOLEAN lost; // True if a robot is lost
  CPU_BOOLEAN dead; // True if a robot has died in a collision
  CPU_BOOLEAN loop; //If true loop until stop cmd
  Position currentPos; // Current x-y position of a robot
  Position destination; // Destination x-y position for a robot
  CPU_INT32U stepCnt; // Step number of a moving robot
  OS_Q myQueue; // This robot's queue
  OS_Q myMbox; // This robot's mailbox
  OS_SEM msgWaiting; // Indicates a message is waiting in
  CPU_STK rStack[ROBOT_STK_SIZE]; // Stack for this Robot Control Task
  OS_TCB robotTCB; // Task Control Block
} RobotStatus;

/*----- f u n c t i o n    p r o t o t y p e s -----*/
void CreateRobotTask(RobotStatus* robotStatus);

//Scope global robot status array
static RobotStatus robotStatuses[NUM_ROBOTS]; // Array giving status for each robot

#endif