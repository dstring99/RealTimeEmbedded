#include "includes.h"
#include "assert.h"
#include "Globals.h"
#include "Buffer.h"
#include "MemMgr.h"
#include "framer.h"
#include "RobotCtrl.h"
#include "RobotLocMgr.h"
#include "RobotMgr.h"

/* Size of the stacks for each task */
#define	ROBOT_STK_SIZE     256 
#define RBT_IDX_OFFSET 3

CPU_STK rStack[ROBOT_STK_SIZE];

/*----- g l o b a l s -----*/

void RobotTask(void *data);
void CreateStepPayload(Payload* inputPayload, Payload* outputPayload, RobotStatus* robotStatus);
void SetDirection(Payload* inputPayload, Payload* outputPayload, RobotStatus* robotStatus);

void CreateRobotTask(RobotStatus* robotStatus)
{
  OS_ERR osErr;/* -- uCos Error Code */   
  
  // Create the Queue.
  OSQCreate(&(robotStatus->myQueue), "Robot Queue", PoolSize, &osErr);
  assert(osErr == OS_ERR_NONE);
  
  // Create the Queue.
  OSQCreate(&(robotStatus->myMbox), "Robot Mailbox", PoolSize, &osErr);
  assert(osErr == OS_ERR_NONE);
  
  robotStatus->loop = FALSE;

  /* Create the Input task. */	
  OSTaskCreate(&(robotStatus->robotTCB),
               "Robot Task",
               RobotTask, 
               (void*)robotStatus,
               RobotPrio,
               robotStatus->rStack,
               ROBOT_STK_SIZE / 10,
               ROBOT_STK_SIZE,
               0,
               0,
               0,
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               &osErr);
  assert(osErr == OS_ERR_NONE);
}

void RobotTask(void *data)
{
  OS_ERR osErr;/* -- uCos Error Code */   
  Payload* inputPayload = NULL;
  Payload* outputPayload = NULL;
  Payload* hereIamPayload = NULL;
  OS_MSG_SIZE msgSize;
  RobotStatus* robotStatus = (RobotStatus*)data;
  CPU_INT08S x = 0;
  CPU_INT08S y = 0;
  
  while(1)
  {
    if(inputPayload == NULL)
    {
      inputPayload = OSQPend(&(robotStatus->myQueue),0, OS_OPT_PEND_BLOCKING, &msgSize, NULL,&osErr);
      assert(osErr==OS_ERR_NONE);
    }
    
    if (outputPayload == NULL)
    {
      outputPayload = Allocate();
    }
    
    OSSemPend(&openDirTest, 0, OS_OPT_PEND_BLOCKING, NULL, &osErr);
    assert(osErr==OS_ERR_NONE);
    CreateStepPayload(inputPayload, outputPayload, robotStatus);
    OSSemPost(&openDirTest, OS_OPT_POST_1, &osErr);
    assert(osErr==OS_ERR_NONE);
    
    if(outputPayload->dataPart.Step.Direction)
    {
      OSQPost(&framerQueue, outputPayload, sizeof(Payload), OS_OPT_POST_FIFO, &osErr);
      assert(osErr==OS_ERR_NONE);
    }
    else
    {
      //return payload to pool
      Free(outputPayload);
    }
    outputPayload = NULL;
    
    while(1)
    {
      if(hereIamPayload == NULL)
      {
        hereIamPayload = OSQPend(&(robotStatus->myMbox),0, OS_OPT_PEND_BLOCKING, &msgSize, NULL,&osErr);
        assert(osErr==OS_ERR_NONE);
      }
      
      if (outputPayload == NULL)
      {
        outputPayload = Allocate();
      }
      
      //OSSemPend(&openDirTest, 0, OS_OPT_PEND_BLOCKING, NULL, &osErr);
      //assert(osErr==OS_ERR_NONE);
      CreateStepPayload(inputPayload, outputPayload, robotStatus);
      //OSSemPost(&openDirTest, OS_OPT_POST_1, &osErr);
      //assert(osErr==OS_ERR_NONE);
      
      if(outputPayload->dataPart.Step.Direction)
      {
        //locationMatrix[robotStatus->currentPos.x][robotStatus->currentPos.y] = FALSE;
        x = robotStatus->currentPos.x + offsetMatrix[outputPayload->dataPart.Step.Direction][0];
        y = robotStatus->currentPos.y + offsetMatrix[outputPayload->dataPart.Step.Direction][1];
        locationMatrix[x][y] = TRUE;
        
        OSQPost(&framerQueue, outputPayload, sizeof(Payload), OS_OPT_POST_FIFO, &osErr);
        assert(osErr==OS_ERR_NONE);
        
        outputPayload = NULL;
        Free(hereIamPayload);
        hereIamPayload = NULL;
      }
      else
      {
        //return payloads to pool
        Free(outputPayload);
        outputPayload = NULL;
        Free(hereIamPayload);
        hereIamPayload = NULL;
        break;
      }
    }
    
    if(robotStatus->loop)
    {
      //If looping put move command back onto queue
      OSQPost(&(robotStatus->myQueue), inputPayload, sizeof(Payload), OS_OPT_POST_FIFO, &osErr);
      assert(osErr==OS_ERR_NONE);
    }
    else
    {
      Free(inputPayload);
    }
    inputPayload = NULL;
  }
}

void CreateStepPayload(Payload* inputPayload, Payload* outputPayload, RobotStatus* robotStatus)
{
  outputPayload->payloadLen = STEP_LEN;
  
  outputPayload->dstAddr = inputPayload->srcAddr;
  if(inputPayload->msgType == MV_TYPE)
  {
    outputPayload->dstAddr = inputPayload->dataPart.AddMove.address;
  }
  
  outputPayload->srcAddr = RBT_MGR_ADDR;
  outputPayload->msgType = STEP_TYPE;
  SetDirection(inputPayload, outputPayload, robotStatus);
}

//return clockwise dir
CPU_INT08U cw(CPU_INT08U dir)
{
  if (dir == 0) return 0;
  dir = (dir + 1) % 9;
  if (dir == 0) dir = 1;
  return dir;
}

//return counter cw dir
CPU_INT08U ccw(CPU_INT08U dir)
{
  if (dir == 0) return 0;
  dir = (dir - 1) % 9;
  if (dir == 0) dir = 8;
  return dir;
}


void TryDirections(Position* pos, CPU_INT08U* dir)
{
  CPU_INT08U dirOrig = *dir;
  //try clockwise dir
  if(TestDirection(*pos, *dir)) *dir = cw(dirOrig);
  else return;
  
  //try counter clockwise dir
  if(TestDirection(*pos, *dir)) *dir = ccw(dirOrig);
  else return;
  
  //stay put
  if(TestDirection(*pos, *dir)) *dir = DIR_0;
}

//sets direction for a step command based on position and destination
void SetDirection(Payload* inputPayload, Payload* outputPayload, RobotStatus* robotStatus)
{
  //OS_ERR osErr;
  
  //set destination if first step
  if(inputPayload->msgType == MV_TYPE)
  {
    robotStatus->destination = inputPayload->dataPart.AddMove.position;
  }
  
  //compute differences in x and y coords
  CPU_INT08S deltaX = robotStatus->destination.x - robotStatus->currentPos.x;
  CPU_INT08S deltaY = robotStatus->destination.y - robotStatus->currentPos.y;
  
  
  //use deltas to return appropriate direction
  //North
  if(deltaX == 0 && deltaY > 0)
  {
    outputPayload->dataPart.Step.Direction = DIR_N;
    TryDirections(&(robotStatus->currentPos), &(outputPayload->dataPart.Step.Direction));
    return;
  }
  
  //Northeast
  if(deltaX > 0 && deltaY > 0)
  {
    outputPayload->dataPart.Step.Direction = DIR_NE;
    TryDirections(&(robotStatus->currentPos), &(outputPayload->dataPart.Step.Direction));
    return;
  }
  
  //East
  if(deltaX > 0 && deltaY == 0)
  {
    outputPayload->dataPart.Step.Direction = DIR_E;
    TryDirections(&(robotStatus->currentPos), &(outputPayload->dataPart.Step.Direction));
    return;
  }
  
  //Southeast
  if(deltaX > 0 && deltaY < 0)
  {
    outputPayload->dataPart.Step.Direction = DIR_SE;
    TryDirections(&(robotStatus->currentPos), &(outputPayload->dataPart.Step.Direction));
    return;
  }
  
  //South
  if(deltaX == 0 && deltaY < 0)
  {
    outputPayload->dataPart.Step.Direction = DIR_S;
    TryDirections(&(robotStatus->currentPos), &(outputPayload->dataPart.Step.Direction));
    return;
  }
  
  //Southwest
  if(deltaX < 0 && deltaY < 0)
  {
    outputPayload->dataPart.Step.Direction = DIR_SW;
    TryDirections(&(robotStatus->currentPos), &(outputPayload->dataPart.Step.Direction));
    return;
  }
  
  //West
  if(deltaX < 0 && deltaY == 0)
  {
    outputPayload->dataPart.Step.Direction = DIR_W;
    TryDirections(&(robotStatus->currentPos), &(outputPayload->dataPart.Step.Direction));
    return;
  }
  
  //NorthWest
  if(deltaX < 0 && deltaY > 0)
  {
    outputPayload->dataPart.Step.Direction = DIR_NW;
    TryDirections(&(robotStatus->currentPos), &(outputPayload->dataPart.Step.Direction));
    return;
  }
  
  //Stay put
  outputPayload->dataPart.Step.Direction = DIR_0;
}