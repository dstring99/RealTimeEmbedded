#ifndef BFRPAIR_H
#define BFRPAIR_H

#include "includes.h"
#include "Buffer.h"

#define NumBfrs 2
typedef struct
{
  CPU_INT08U putBfrNum; /* -- The index of the put buffer (either 0 or 1) */
  Buffer buffers[NumBfrs]; /* -- The buffer pair’s 2 buffers */
} BfrPair;

static OS_SEM	openGetBfr;

void BfrPairInit(BfrPair *bfrPair, CPU_INT08U *bfr0Space, CPU_INT08U *bfr1Space, CPU_INT16U size);
void PutBfrReset(BfrPair *bfrPair); 
CPU_INT08U *PutBfrDataSpace(BfrPair *bfrPair); 
CPU_INT08U *GetBfrDataSpace(BfrPair *bfrPair); 
CPU_BOOLEAN PutBfrIsClosed(BfrPair *bfrPair); 
CPU_BOOLEAN GetBfrIsClosed(BfrPair *bfrPair);
CPU_BOOLEAN PutBfrNotEmpty(BfrPair *bfrPair);
CPU_BOOLEAN GetBfrEmpty(BfrPair *bfrPair);
void PutBfrClose(BfrPair *bfrPair); 
void GetBfrOpen (BfrPair *bfrPair); 
CPU_INT16S PutBfrAddByte(BfrPair *bfrPair, CPU_INT16S byte);
CPU_INT16S GetBfrNextByte(BfrPair *bfrPair);
CPU_INT16S GetBfrRemByte(BfrPair *bfrPair);
CPU_BOOLEAN BfrPairSwappable(BfrPair*bfrPair);
void BfrPairSwap(BfrPair *bfrPair);
#endif