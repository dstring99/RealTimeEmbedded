#ifndef BUFFER_H
#define BUFFER_H

#include "includes.h"

#ifndef BfrSize
#define BfrSize 4
#endif

typedef struct
{
  volatile CPU_BOOLEAN closed; /* -- True if buffer has data ready to process: is ready to be emptied or being emptied */
  /* -- False if buffer is not ready to process: is ready to fill or being filled */
  CPU_INT16U size; /* -- The capacity of the buffer in bytes */
  CPU_INT16U putIndex; /* -- The position where the next byte is added */
  CPU_INT16U getIndex; /* -- The position of the next byte to remove */
  CPU_INT08U *buffer; /* -- The address of an array of bytes serving as the buffer’s data space */
} Buffer;

void BfrInit(Buffer *bfr, CPU_INT08U *bfrSpace, CPU_INT16U size);
void BfrReset(Buffer *bfr);
CPU_BOOLEAN BfrClosed(Buffer *bfr);
void BfrClose(Buffer *bfr); 
void BfrOpen(Buffer *bfr); 
CPU_BOOLEAN BfrFull(Buffer *bfr);
CPU_BOOLEAN BfrEmpty(Buffer *bfr);
CPU_INT16S BfrAddByte(Buffer *bfr, CPU_INT16S theByte);
CPU_INT16S BfrNextByte(Buffer *bfr); 
CPU_INT16S BfrRemByte(Buffer *bfr);

#endif