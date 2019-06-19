/*=============== P k t P a r s e r . c ===============*/

#include "PktParser.h"
#include "includes.h"
#include "Error.h"
#include "payload.h"
#include "MemMgr.h"
#include "RobotMgr.h"
#include "Framer.h"


//----- c o n s t a n t    d e f i n i t i o n s -----

/* Size of the Process task stack */
#define	PARSER_STK_SIZE     256 
#define DATA_ARR_LEN 1

typedef struct
{
  CPU_INT08S payloadLen;
  CPU_INT08U data[DATA_ARR_LEN];
} PktBfr;

typedef enum {Sync0, Sync1, Sync2, Len, Pload, Err} ParserState;

Payload payloadBfr;
   
/*----- g l o b a l s -----*/

// Process Task Control Block
OS_TCB parserTCB;

/* Stack space for Process task stack */
CPU_STK	parserStk[PARSER_STK_SIZE];

//----- f u n c t i o n    p r o t o t y p e s -----
CPU_VOID ParsePkt(CPU_VOID *data);

//Semaphore counters
OS_SEM_CTR open, closed;

/* Parser queue */
OS_Q	parserQueue;

/*----- C r e a t e P a r s e r T a s k ( ) -----

PURPOSE
Create and initialize the Parser Task.
*/
CPU_VOID CreateParserTask(CPU_VOID)
{
  OS_ERR osErr;/* -- OS Error code */
  
  // Create the Queue.
  OSQCreate(&parserQueue, "Parser Queue", PoolSize, &osErr);
  assert(osErr == OS_ERR_NONE);

  /* Create Parser task. */	
  OSTaskCreate(&parserTCB,
               "Processing Task",
               ParsePkt, 
               (void*) &payloadBfr,
               ParserPrio,
               &parserStk[0],
               PARSER_STK_SIZE / 10,
               PARSER_STK_SIZE,
               0,
               0,
               0,
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               &osErr);

  assert(osErr == OS_ERR_NONE);
}

//function to read bytes one by one and create packet buffer if valid
//void ParsePkt(void *payloadBfr)
CPU_VOID ParsePkt(CPU_VOID *payloadBfr)
{
  ParserState parserState = Sync0;
  CPU_INT08U c;
  CPU_INT08S i=0;
  CPU_INT08U checkSum = 0;
  CPU_INT08U errorFound = 0;
  OS_ERR osErr;
            
  PktBfr *pktBfr = NULL;
  Payload *errorBfr = NULL;
  
  while(1)
  {
    if (pktBfr == NULL)
      pktBfr = (PktBfr *)Allocate();
    
    c = GetByte();
    checkSum ^= c;
    
    //check packet validity based on parser state
    switch (parserState)
    {
      case Sync0:
      {
        pktBfr->payloadLen = 0;
        parserState = Sync1;
        errorFound = ValidateSyncByte(c, 0);
        break;
        //always proceed to Sync1
      }
      case Sync1:
      {
        parserState = Sync2;
        if(!errorFound)
          errorFound = ValidateSyncByte(c, 1);
        break;
        //always proceed to Sync2
      }
      case Sync2:
      {
        parserState = Len;
        if(!errorFound)
          errorFound = ValidateSyncByte(c, 2);
        break;
        //always proceed to check Len byte
      }
      case Len:
        {
          parserState = Pload;
          pktBfr->payloadLen = c - HEADER_LEN;
          
          if(!errorFound)
          {
            errorFound = ValidatePacketLen(c);
            if(errorFound)
            {
              parserState =  Err;
              errorBfr = Allocate();
              CreateErrorPacket(errorBfr, errorFound);
              OSQPost(&framerQueue, errorBfr, sizeof(Payload), OS_OPT_POST_FIFO, &osErr);
              assert(osErr==OS_ERR_NONE);
              errorBfr = NULL;
              break;
            }
          }

          break;
        }
      case Pload:
      {    
          pktBfr->data[i++] = c;
          if(i >= pktBfr->payloadLen)
          {
            if(!errorFound)
              errorFound = ValidateChecksum(checkSum);
            
            if(errorFound)
            {
              CreateErrorPacket((Payload *)pktBfr, errorFound);
              OSQPost(&framerQueue, (Payload *)pktBfr, sizeof(Payload), OS_OPT_POST_FIFO, &osErr);
              assert(osErr==OS_ERR_NONE);
            }
            else
            {
              OSQPost(&parserQueue, (Payload *)pktBfr, sizeof(Payload), OS_OPT_POST_FIFO, &osErr);
              assert(osErr==OS_ERR_NONE);
            }
            
            pktBfr = NULL;
            parserState = Sync0;
            i=0;
            checkSum = 0;
          }
          break;
      }
      case Err:
      {
        //Only used for bad packet Len
        //Remain in Err state until sync triplet is found.
        if(FindSyncTriplet(c))
        {
          i=0;
          parserState = Len;
          pktBfr = NULL;
          checkSum = SYNC_BYTE_0 ^ SYNC_BYTE_1 ^ SYNC_BYTE_2;
          errorFound = 0;
        }
        else
        {
          checkSum = 0;
        }
        break;
      }
    }
  }
}