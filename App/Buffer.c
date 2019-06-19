#include "Buffer.h"

//initializes buffer
void BfrInit(Buffer *bfr, CPU_INT08U *bfrSpace, CPU_INT16U size)
{
  bfr->buffer = bfrSpace;
  bfr->size = size;
  bfr->closed = FALSE;
  bfr->putIndex = 0;
  bfr->getIndex = 0;
}

//resets buffer
void BfrReset(Buffer *bfr)
{
  bfr->closed = FALSE;
  bfr->putIndex = 0;
  bfr->getIndex = 0;
}

//returns buffer closed state
CPU_BOOLEAN BfrClosed(Buffer *bfr){
  return bfr->closed;
}

//closes the buffer
void BfrClose(Buffer *bfr)
{
  bfr->closed = TRUE;
}

//opens the buffer
void BfrOpen(Buffer *bfr)
{
  bfr->closed = FALSE;
}

//returns full state
CPU_BOOLEAN BfrFull(Buffer *bfr)
{
  return bfr->putIndex >= bfr->size;
}

//returns empty state
CPU_BOOLEAN BfrEmpty(Buffer *bfr)
{
  return (bfr->getIndex >= bfr->putIndex);
}

//adds byte and returns it if there is room, returns -1 if not
CPU_INT16S BfrAddByte(Buffer *bfr, CPU_INT16S theByte)
{
  if(BfrFull(bfr))
    return -1;
  
  bfr->buffer[(bfr->putIndex)++] = theByte;
  
  if(BfrFull(bfr))
    BfrClose(bfr);
  
  return theByte;
}

//returns next byte w/o removing it
CPU_INT16S BfrNextByte(Buffer *bfr)
{
  if(BfrEmpty(bfr))
     return -1;
     
  return bfr->buffer[bfr->getIndex];
}

//removes and returns next byte, returns -1 if empty
CPU_INT16S BfrRemByte(Buffer *bfr)
{
  if(BfrEmpty(bfr))
     return -1;
  
  CPU_INT16S c = bfr->buffer[bfr->getIndex++];
  if(BfrEmpty(bfr))
    BfrReset(bfr);
  
  return c;
}