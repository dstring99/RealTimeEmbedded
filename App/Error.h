/*
Dan Stringer
Error.h
*/

#ifndef ERROR_H
#define ERROR_H

#include "Payload.h"

CPU_INT08U ValidateSyncByte(CPU_INT08U c, CPU_INT08U syncByte);
CPU_INT08U ValidatePacketLen(CPU_INT08U c);
CPU_INT08U ValidateChecksum(CPU_INT08U checkSum);
CPU_BOOLEAN FindSyncTriplet(CPU_INT08U c);
CPU_BOOLEAN ValidateAddress(CPU_INT08U addr);
CPU_BOOLEAN ValidatePosition(Position position);
void CreateErrorPacket(Payload* errorBfr, CPU_INT08U errorFound);
void SendError(CPU_INT08U errorFound);

#endif
