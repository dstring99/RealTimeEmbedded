/*=============== P a y l o a d . h ===============*/
#ifndef PAYLOAD_H
#define PAYLOAD_H

#include <stdio.h>
#include <stdlib.h>
#include "includes.h"
#include "SerIODriver.h"
#include "assert.h"
#include "BfrPair.h"
#include "Globals.h"

/*----- c o n s t a n t d e f i n i t i o n s -----*/
#define MaxPayload 14 // Maximum bytes in a payload
#define PAYLOAD_STK_SIZE 256
#define PayloadPrio 5 // Parser task priority

#define LOOP_PATH_ARR_LEN 20

typedef struct
{
  CPU_INT08U payloadLen;
  CPU_INT08U dstAddr;
  CPU_INT08U srcAddr;
  CPU_INT08U msgType;
  union
  {
    struct
    {
      CPU_INT08U address;
      Position position;
    } AddMove;
    struct
    {
      CPU_INT08U address;
      Position position[LOOP_PATH_ARR_LEN];
    } LoopPath;
    struct
    {
      CPU_INT08U address;
    } Stop;
    struct
    {
      CPU_INT08U Direction;
    } Step;
    struct
    {
      Position position;
    } HereIAm;
    struct
    {
      CPU_INT08U msgType;
    } Ack;
    struct
    {
      CPU_INT08U errorCode;
    } Err;
  } dataPart;
} Payload;

#endif