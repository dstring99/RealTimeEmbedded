#include "BfrPair.h"

//Sets buffer spaces of specified buffer pair
void BfrPairInit( BfrPair *bfrPair, CPU_INT08U *bfr0Space, CPU_INT08U *bfr1Space, CPU_INT16U size)
{
  BfrInit(&(bfrPair->buffers[0]), bfr0Space, size);
  BfrInit(&(bfrPair->buffers[1]), bfr1Space, size);
  bfrPair->putBfrNum = 0;
}

//Resets put buffer
void PutBfrReset(BfrPair *bfrPair)
{
  BfrReset(&(bfrPair->buffers[bfrPair->putBfrNum]));
}

//returns put buffer data space
CPU_INT08U *PutBfrDataSpace(BfrPair *bfrPair)
{
  return bfrPair->buffers[bfrPair->putBfrNum].buffer;
}

//retruns get buffer data space 
CPU_INT08U *GetBfrDataSpace(BfrPair *bfrPair)
{
  return bfrPair->buffers[!(bfrPair->putBfrNum)].buffer;
}

//returns closed status of put buffer
CPU_BOOLEAN PutBfrIsClosed(BfrPair *bfrPair)
{
  return BfrClosed(&(bfrPair->buffers[bfrPair->putBfrNum]));
}

//returns closed status of get buffer
CPU_BOOLEAN GetBfrIsClosed(BfrPair *bfrPair)
{
  return BfrClosed(&(bfrPair->buffers[!(bfrPair->putBfrNum)]));
}

//returns true if put buffer is NOT empty, false otherwise
CPU_BOOLEAN PutBfrNotEmpty(BfrPair *bfrPair)
{
  return !BfrEmpty(&(bfrPair->buffers[bfrPair->putBfrNum]));
}

//returns true if get buffer is empty, false otherwise
CPU_BOOLEAN GetBfrEmpty(BfrPair *bfrPair)
{
  return BfrEmpty(&(bfrPair->buffers[!bfrPair->putBfrNum]));
}

//closes the put buffer
void PutBfrClose(BfrPair *bfrPair)
{
  BfrClose(&(bfrPair->buffers[bfrPair->putBfrNum]));
}

//opens the get buffer
void GetBfrOpen (BfrPair *bfrPair)
{
  BfrOpen(&(bfrPair->buffers[!(bfrPair->putBfrNum)]));
}

//adds byte to the put buffer
CPU_INT16S PutBfrAddByte(BfrPair *bfrPair, CPU_INT16S byte)
{
  return BfrAddByte(&(bfrPair->buffers[bfrPair->putBfrNum]), byte);
}

//returns next byte in get buffer without removing it
CPU_INT16S GetBfrNextByte(BfrPair *bfrPair)
{
  return BfrNextByte(&(bfrPair->buffers[!(bfrPair->putBfrNum)]));
}

//removes and returns get bfr byte
CPU_INT16S GetBfrRemByte(BfrPair *bfrPair)
{
  return BfrRemByte(&(bfrPair->buffers[!(bfrPair->putBfrNum)]));
}

//returns flag if buffers are swappable
CPU_BOOLEAN BfrPairSwappable(BfrPair*bfrPair)
{
  return bfrPair->buffers[bfrPair->putBfrNum].closed && !(bfrPair->buffers[!(bfrPair->putBfrNum)].closed);
}

//performs put and get buffer swap
void BfrPairSwap(BfrPair *bfrPair)
{
  bfrPair->putBfrNum = !(bfrPair->putBfrNum);
  BfrReset(&(bfrPair->buffers[bfrPair->putBfrNum]));
}
