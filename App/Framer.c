/*=============== RobotMgr.c ===============*/

#include "includes.h"
#include "assert.h"
#include "Globals.h"
#include "Buffer.h"
#include "MemMgr.h"
#include "SerIODriver.h"

/*----- c o n s t a n t    d e f i n i t i o n s -----*/

/* Size of the stacks for each task */
#define	FRAMER_STK_SIZE     256 
#define PREAMBLE_LEN 3

#define DATA_ARR_LEN 8

/*----- g l o b a l s -----*/

// Task Control Block
OS_TCB framerTCB;

/* task stack space. */
static CPU_STK	framerStack[FRAMER_STK_SIZE]; 

/* Input queue */
OS_Q framerQueue;

//Message Preamble
CPU_INT08U msgPreamble[PREAMBLE_LEN] = { SYNC_BYTE_0, SYNC_BYTE_1, SYNC_BYTE_2};


/*----- f u n c t i o n    p r o t o t y p e s -----*/
void FramerTask(void *data);
void CreateAckBuffer(CPU_INT08U* framerBfr, Payload* payload);
void CreateStepBuffer(CPU_INT08U* framerBfr, Payload* payload);
void CreateErrorBuffer(CPU_INT08U* framerBfr, Payload* payload);
void SetPreamble(CPU_INT08U* framerBfr);

/*--------------- CreateFramerTask ( ) ---------------*/
void CreateFramerTask(void)
{
  OS_ERR osErr;/* -- uCos Error Code */ 
  
  // Create the Queue.
  OSQCreate(&framerQueue, "Framer Queue", PoolSize, &osErr);
  assert(osErr == OS_ERR_NONE);

  /* Create the Input task. */	
  OSTaskCreate(&framerTCB,
               "Framer Task",
               FramerTask, 
               NULL,
               FramerPrio,
               &framerStack[0],
               FRAMER_STK_SIZE / 10,
               FRAMER_STK_SIZE,
               0,
               0,
               0,
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               &osErr);
  assert(osErr == OS_ERR_NONE);
}

/*--------------- FramerTask ( ) ---------------*/
void FramerTask (void *data)
{
  Payload *payload = NULL;
  OS_ERR osErr;
  OS_MSG_SIZE msgSize;
  CPU_INT08U chkSum = 0;
  CPU_INT08U framerBfr[DATA_ARR_LEN];
  CPU_BOOLEAN resetFramerBfr = TRUE;
  CPU_INT08U pktIdx = 0;
  CPU_INT08U pktLen = 0;
  CPU_INT08U c;
  
  /* Task runs forever, or until preempted or suspended. */
  for(;;)
  {
    if(payload == NULL)
    {
      /* If no buffer assigned, get one from the pool. */	
      payload = OSQPend(&framerQueue,
                     0, 
                     OS_OPT_PEND_BLOCKING, 
                     &msgSize, 
                     NULL, 
                     &osErr);
      assert(osErr==OS_ERR_NONE);
    }
    
    if(resetFramerBfr)
    {
      resetFramerBfr = FALSE;
      
      switch(payload->msgType)
      {
        case(ACK_TYPE):
        {
          CreateAckBuffer(framerBfr, payload);
          pktLen = ACK_LEN + HEADER_LEN - 1;
          break;
        }
        case(STEP_TYPE):
        {
          CreateStepBuffer(framerBfr, payload);
          pktLen = STEP_LEN + HEADER_LEN - 1;
          break;
        }
        case(ERR_TYPE):
        {
          CreateErrorBuffer(framerBfr, payload);
          pktLen = ERR_LEN + HEADER_LEN - 1;
          break;
        }  
      }
    }
    
    c = framerBfr[pktIdx++];
    PutByte(c);
    chkSum ^= c;
    
    if(pktIdx >= pktLen)
    {
      PutByte(chkSum);
      ForceSend();
      chkSum = 0;
      Free(payload);
      payload = NULL;
      resetFramerBfr = TRUE;
      pktIdx = 0;
    }
  }
}

CPU_INT08U PutPreamble()
{
  CPU_INT08U chkSum=0;
  for(int i=0; i<PREAMBLE_LEN; i++)
  {
    PutByte(msgPreamble[i]);
    chkSum ^= msgPreamble[i];
  }
  return chkSum;
}

void CreateAckBuffer(CPU_INT08U* framerBfr, Payload* payload)
{
  SetPreamble(framerBfr);
  framerBfr[3] = ACK_LEN + HEADER_LEN;
  framerBfr[4] = CTRL_CENT_ADDR;
  framerBfr[5] = RBT_MGR_ADDR;
  framerBfr[6] = ACK_TYPE;
  framerBfr[7] = payload->dataPart.Ack.msgType;
}

void CreateStepBuffer(CPU_INT08U* framerBfr, Payload* payload)
{
  SetPreamble(framerBfr);
  framerBfr[3] = STEP_LEN + HEADER_LEN;
  framerBfr[4] = payload->dstAddr;
  framerBfr[5] = RBT_MGR_ADDR;
  framerBfr[6] = STEP_TYPE;
  framerBfr[7] = payload->dataPart.Step.Direction;
}

void CreateErrorBuffer(CPU_INT08U* framerBfr, Payload* payload)
{
  SetPreamble(framerBfr);
  framerBfr[3] = ERR_LEN + HEADER_LEN;
  framerBfr[4] = CTRL_CENT_ADDR;
  framerBfr[5] = RBT_MGR_ADDR;
  framerBfr[6] = ERR_TYPE;
  framerBfr[7] = payload->dataPart.Err.errorCode;
}

void SetPreamble(CPU_INT08U* framerBfr)
{
  framerBfr[0] = SYNC_BYTE_0;
  framerBfr[1] = SYNC_BYTE_1;
  framerBfr[2] = SYNC_BYTE_2;
}
