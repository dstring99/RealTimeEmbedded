/*
Dan Stringer
Error.c
*/

#include "includes.h"
#include "Error.h"
#include "SerIODriver.h"
#include "Globals.h"
#include "Payload.h"
#include "Framer.h"
#include "MemMgr.h"

CPU_INT08U preambleBytes[3] = {SYNC_BYTE_0, SYNC_BYTE_1, SYNC_BYTE_2};

#define MIN_PACKET_LEN 8

//Validate sync byte by comparing it to expected value
CPU_INT08U ValidateSyncByte(CPU_INT08U c, CPU_INT08U syncByteIndex)
{
  return c == preambleBytes[syncByteIndex] ? 0 : syncByteIndex + 1;
}

//Ensure Packet length is valid
CPU_INT08U ValidatePacketLen(CPU_INT08U c)
{
  return c >= MIN_PACKET_LEN ? 0 : ERR_CODE_PKT_LEN;
}

//Perform checksum on end of packet
CPU_INT08U ValidateChecksum(CPU_INT08U checkSum)
{
  return checkSum == 0 ? 0 : ERR_CODE_CHK_SUM;
} 

CPU_BOOLEAN ValidateAddress(CPU_INT08U addr)
{
  return addr < ADDR_MIN || addr > ADDR_MAX;
}

CPU_BOOLEAN ValidatePosition(Position position)
{
  return position.x > POS_XMAX || position.x < POS_XMIN || position.y > POS_YMAX || position.y < POS_YMIN;
}

//if length is invalid, look for sync triplet
CPU_BOOLEAN FindSyncTriplet(CPU_INT08U c)
{
  if(c == SYNC_BYTE_0)
  {
    c = GetByte();
    
    if(c == SYNC_BYTE_1)
    {
      c = GetByte(); 
  
      if(c == SYNC_BYTE_2)
      {
        return 1;
      }
    }
  }
  return 0;
}

//Create error packet
void CreateErrorPacket(Payload* errorBfr, CPU_INT08U errorFound)
{
  errorBfr->payloadLen = ERR_LEN;
  errorBfr->dstAddr = CTRL_CENT_ADDR;
  errorBfr->srcAddr = RBT_MGR_ADDR;
  errorBfr->msgType = ERR_TYPE;
  errorBfr->dataPart.Err.errorCode = errorFound;
}

//Post error msg to framer Queue
void SendError(CPU_INT08U errorFound)
{
  OS_ERR osErr;
  Payload* errorPayload = Allocate();
  CreateErrorPacket(errorPayload, errorFound);
  OSQPost(&framerQueue, errorPayload, sizeof(Payload), OS_OPT_POST_FIFO, &osErr);
  assert(osErr==OS_ERR_NONE);
}