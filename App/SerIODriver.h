#ifndef SERIODRIVER_H
#define SERIODRIVER_H

#include "includes.h"

// Create and Initialize iBfrPair and oBfrPair.
void InitSerIO();
    
// Service the RS232 receiver.
void ServiceRx();

// Service the RS232 transmitter.
void ServiceTx();

CPU_INT16S GetByte();

CPU_INT16S PutByte(CPU_INT16S c);

void ForceSend();

void SerialISR(void);

#endif