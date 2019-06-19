/*=============== RobotMgr.c ===============*/

#include "includes.h"
#include "assert.h"
#include "Globals.h"
#include "Buffer.h"
#include "MemMgr.h"
#include "SerIODriver.h"
#include "PktParser.h"
#include "Framer.h"
#include "RobotCtrl.h"
#include "RobotMgr.h"
#include "RobotLocMgr.h"
#include "Error.h"

/*----- c o n s t a n t    d e f i n i t i o n s -----*/

/* Size of the stacks for each task */
#define	ROBOTMGR_STK_SIZE     256 
#define NUM_ROBOTS 13
#define RBT_IDX_OFFSET 3

/*----- g l o b a l s -----*/

// Robot Manager Task Control Blocks
OS_TCB robotMgrTCB;

/* Robot Manager task stack space. */
static CPU_STK	robotMgrStack[ROBOTMGR_STK_SIZE];

//Robots that already exist
static CPU_BOOLEAN robotExists[NUM_ROBOTS] = {FALSE};

//Robot location matrix
extern CPU_BOOLEAN locationMatrix[POS_XMAX+1][POS_YMAX+1];

//Semaphores to lock and release dir test
OS_SEM openDirTest;


/* Parser queue */
extern OS_Q parserQueue;

/*----- f u n c t i o n    p r o t o t y p e s -----*/
void RobotManagerTask(void *data);
void CreateAckPayload(Payload* payload, Payload* outputPayload );
void AddRobot(Payload* inputPayload, Payload* ackPayload, CPU_INT08U rbtStatusIndex);
void MoveRobot(Payload* inputPayload, Payload* ackPayload, CPU_INT08U rbtStatusIndex);
void TakeNextStep(Payload* inputPayload, CPU_INT08U rbtStatusIndex);
void SetRobotPath(Payload* inputPayload, Payload* ackPayload, CPU_INT08U rbtStatusIndex);
void CreateMovePayload(Payload* movePayload, Payload* inputPayload, CPU_INT08U posIdx);
void StopRobot(Payload* inputPayload, Payload* ackPayload, CPU_INT08U rbtStatusIndex);
void UpdateRobotPosition(Payload* inputPayload, CPU_INT08U rbtStatusIndex);
CPU_BOOLEAN ValidatePositions(Payload* inputPayload);

void CreateRobotMgrTask(void)
{
  OS_ERR osErr;/* -- uCos Error Code */   
  
  // Create the Open Output Buffer semaphore.
  OSSemCreate(&openDirTest, "Open Direction Test", 13, &osErr);
  assert(osErr == OS_ERR_NONE);

  /* Create the Robot Manager task. */	
  OSTaskCreate(&robotMgrTCB,
               "Robot Manager Task",
               RobotManagerTask, 
               NULL,
               RobotMgrPrio,
               &robotMgrStack[0],
               ROBOTMGR_STK_SIZE / 10,
               ROBOTMGR_STK_SIZE,
               0,
               0,
               0,
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               &osErr);
  assert(osErr == OS_ERR_NONE);
}

/*--------------- RobotManagerTask( ) ---------------*/
void RobotManagerTask(void *data)
{
  Payload *inputPayload = NULL;	/* -- Current input buffer */
  Payload *ackPayload = NULL;	/* -- Current output buffer */
  
  OS_ERR osErr;        /* -- uCos Error Code */   
  OS_MSG_SIZE msgSize;
  CPU_INT08U rbtStatusIndex = 0;
  CPU_INT08U errorFound = 0;
  
  /* Task runs forever, or until preempted or suspended. */
  for(;;)
  {
    /* If no buffer assigned, get one from the pool. */	
    if (inputPayload == NULL)
    {
      inputPayload = OSQPend(&parserQueue,0, OS_OPT_PEND_BLOCKING, &msgSize, NULL,&osErr);
      assert(osErr==OS_ERR_NONE);
      rbtStatusIndex = inputPayload->dataPart.AddMove.address - RBT_IDX_OFFSET;
    }
    
    //Allocate ack payload
    if (ackPayload == NULL && inputPayload->msgType != HERE_TYPE)
    {
      ackPayload = Allocate();
    }
    
    //Use msg type to determine action
    switch(inputPayload->msgType)
    {
      case(AD_TYPE): //Add robot
      {
        if(ValidateAddress(inputPayload->dataPart.AddMove.address))
          errorFound = ERR_AD_BAD_ADDR;
        else if(ValidatePosition(inputPayload->dataPart.AddMove.position))
          errorFound = ERR_AD_BAD_LOC;
        else if(robotExists[rbtStatusIndex])
          errorFound = ERR_AD_RBT_EXIST;
        else if(PositionOccupied(inputPayload->dataPart.AddMove.position))
          errorFound = ERR_AD_LOC_OCC;
        if(errorFound)
          break;
        
        AddRobot(inputPayload, ackPayload, rbtStatusIndex);
        break;
      }
      case(MV_TYPE): //Move robot
      {
        if(ValidateAddress(inputPayload->dataPart.AddMove.address))
          errorFound = ERR_MV_BAD_ADDR;
        else if(ValidatePosition(inputPayload->dataPart.AddMove.position))
          errorFound = ERR_MV_BAD_LOC;
        else if(!robotExists[rbtStatusIndex])
          errorFound = ERR_MV_RBT_NONEXIST;
        if(errorFound)
          break;
        
        MoveRobot(inputPayload, ackPayload, rbtStatusIndex);
        break;
      }
      case(HERE_TYPE): //Here I am says the robot!
      {
        rbtStatusIndex = inputPayload->srcAddr - RBT_IDX_OFFSET;
        UpdateRobotPosition(inputPayload, rbtStatusIndex);
        TakeNextStep(inputPayload, rbtStatusIndex);
        break;
      }
      case(STOP_TYPE): //Stop the robot
      {
        if(ValidateAddress(inputPayload->dataPart.Stop.address))
          errorFound = ERR_ST_BAD_ADDR;
        else if(!robotExists[rbtStatusIndex])
          errorFound = ERR_ST_RBT_NONEXIST;
        if(errorFound)
          break;
        
        StopRobot(inputPayload, ackPayload, rbtStatusIndex);
        break;
      }
      case(LP_TYPE): //Loop command
      {
        if(ValidateAddress(inputPayload->dataPart.LoopPath.address))
          errorFound = ERR_LP_BAD_ADDR;
        else if(ValidatePositions(inputPayload))
          errorFound = ERR_LP_BAD_LOC;
        else if(!robotExists[rbtStatusIndex])
          errorFound = ERR_LP_RBT_NONEXIST;
        if(errorFound)
          break;
        
        robotStatuses[rbtStatusIndex].loop = TRUE;
        //Do not break, fall through to path stage
      }
      case(PA_TYPE): //Path command
      {
        if(ValidateAddress(inputPayload->dataPart.LoopPath.address))
          errorFound = ERR_PA_BAD_ADDR;
        else if(ValidatePositions(inputPayload))
          errorFound = ERR_PA_BAD_LOC;
        else if(!robotExists[rbtStatusIndex])
          errorFound = ERR_PA_RBT_NONEXIST;
        if(errorFound)
          break;
        
        SetRobotPath(inputPayload, ackPayload, rbtStatusIndex);
        break;
      }
      default:
      {
        errorFound = ERR_MSG_TYPE;
        break;
      }
    }
    
    if(errorFound)
    {
      SendError(errorFound);
      Free(inputPayload);
      Free(ackPayload);
    }
    
    //Clear payloads for next iteration
    inputPayload = NULL;
    ackPayload = NULL;
    errorFound = FALSE;
  }
}

//Create Ack Payload from input payload
void CreateAckPayload(Payload* inputPayload, Payload* outputPayload)
{
  outputPayload->payloadLen = ACK_LEN;
  outputPayload->dstAddr = inputPayload->srcAddr;
  outputPayload->srcAddr = inputPayload->dstAddr;
  outputPayload->msgType = ACK_TYPE;
  outputPayload->dataPart.Ack.msgType = inputPayload->msgType;
}

//Start robot task, send ack
void AddRobot(Payload* inputPayload, Payload* ackPayload, CPU_INT08U rbtStatusIndex)
{
  OS_ERR osErr; 
  RobotStatus* status = &(robotStatuses[rbtStatusIndex]);
  
  //Set status address and create task.
  status->address = inputPayload->dataPart.AddMove.address;
  status->currentPos = inputPayload->dataPart.AddMove.position;
  CreateRobotTask(status);
  
  //Send ack payload
  CreateAckPayload(inputPayload, ackPayload);
  OSQPost(&framerQueue, ackPayload, sizeof(Payload), OS_OPT_POST_FIFO, &osErr);
  assert(osErr==OS_ERR_NONE);
  
  //Mark robot as existing and set its location in the loc matrix
  robotExists[rbtStatusIndex] = TRUE;
  locationMatrix[status->currentPos.x][status->currentPos.y] = TRUE;
  
  //return payload to pool
  Free(inputPayload);
}

//Send ack, forward move cmd to robot ctrl via queue
void MoveRobot(Payload* inputPayload, Payload* ackPayload, CPU_INT08U rbtStatusIndex)
{
  OS_ERR osErr; 
  
  //Send ack payload
  CreateAckPayload(inputPayload, ackPayload);
  OSQPost(&framerQueue, ackPayload, sizeof(Payload), OS_OPT_POST_FIFO, &osErr);
  assert(osErr==OS_ERR_NONE);
  
  //Forward mv command to robot control task via its queue
  OSQPost(&(robotStatuses[rbtStatusIndex].myQueue), inputPayload, sizeof(Payload), OS_OPT_POST_FIFO, &osErr);
  assert(osErr==OS_ERR_NONE);
}

//Send ack, add move cmds to robot ctrl via queue
void SetRobotPath(Payload* inputPayload, Payload* ackPayload, CPU_INT08U rbtStatusIndex)
{
  OS_ERR osErr; 
  CPU_INT08U numPts = (inputPayload->payloadLen - 5)/2;
  Payload* movePayload = NULL;
  
  //Send ack payload
  CreateAckPayload(inputPayload, ackPayload);
  OSQPost(&framerQueue, ackPayload, sizeof(Payload), OS_OPT_POST_FIFO, &osErr);
  assert(osErr==OS_ERR_NONE);
  
  for(CPU_INT08U i = 0; i < numPts; i++)
  {
    movePayload = Allocate();
    
    //Create move command from payload data
    CreateMovePayload(movePayload, inputPayload, i);
    
    //Forward mv command to robot control task via its queue
    OSQPost(&(robotStatuses[rbtStatusIndex].myQueue), movePayload, sizeof(Payload), OS_OPT_POST_FIFO, &osErr);
    assert(osErr==OS_ERR_NONE);
    movePayload = NULL;
  }
  
  //return payload to pool
  Free(inputPayload);
}

//Instruct robot to take next step
void TakeNextStep(Payload* inputPayload, CPU_INT08U rbtStatusIndex)
{
  OS_ERR osErr; 
  
  //Forward mv command to robot control task via its queue
  OSQPost(&(robotStatuses[rbtStatusIndex].myMbox), inputPayload, sizeof(Payload), OS_OPT_POST_FIFO, &osErr);
  assert(osErr==OS_ERR_NONE);
}

//On Here I am, update robot position in status struct
void UpdateRobotPosition(Payload* inputPayload, CPU_INT08U rbtStatusIndex)
{
  RobotStatus* status = &(robotStatuses[rbtStatusIndex]);
  locationMatrix[status->currentPos.x][status->currentPos.y] = FALSE;
  status->currentPos = inputPayload->dataPart.HereIAm.position;
  locationMatrix[status->currentPos.x][status->currentPos.y] = TRUE;
}

//Create move payload from input payload
void CreateMovePayload(Payload* movePayload, Payload* inputPayload, CPU_INT08U posIdx)
{
  movePayload->payloadLen = MOVE_LEN;
  movePayload->dstAddr = RBT_MGR_ADDR;
  movePayload->srcAddr = CTRL_CENT_ADDR;
  movePayload->msgType = MV_TYPE;
  movePayload->dataPart.AddMove.address = inputPayload->dataPart.LoopPath.address;
  movePayload->dataPart.AddMove.position = inputPayload->dataPart.LoopPath.position[posIdx];
}

//Instruct robot to stop
void StopRobot(Payload* inputPayload, Payload* ackPayload, CPU_INT08U rbtStatusIndex)
{
  OS_ERR osErr; 
  
  //stop loop command
  robotStatuses[rbtStatusIndex].loop = FALSE;
  
  //Send ack payload
  CreateAckPayload(inputPayload, ackPayload);
  OSQPost(&framerQueue, ackPayload, sizeof(Payload), OS_OPT_POST_FIFO, &osErr);
  assert(osErr==OS_ERR_NONE);
  
  //return payload to pool
  Free(inputPayload);
}

//Validate all pos for path or loop
CPU_BOOLEAN ValidatePositions(Payload* inputPayload)
{
  CPU_INT08U numPts = (inputPayload->payloadLen - 5)/2;
  
  for(CPU_INT08U i = 0; i<numPts; i++)
  {
    if(ValidatePosition(inputPayload->dataPart.LoopPath.position[i]))
       return TRUE;
  }
  return FALSE;
}