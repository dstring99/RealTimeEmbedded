#ifndef PKTPARSER_H
#define PKTPARSER_H

/*=============== P k t P a r s e r . h ===============*/

/*
BY:	George Cheney
		EECE472 / EECE572 Embedded Real Time Systems
		Electrical and Computer Engineering Dept.
		UMASS Lowell
*/

/*
PURPOSE
This is the interface to the module PktParser.c.

CHANGES
02-21-2019  - Created for Spring 2019
*/

#include "includes.h"

#include "BfrPair.h"
#include "SerIODriver.h"
#include "Payload.h"
#include "assert.h"
      
//----- f u n c t i o n    p r o t o t y p e s -----

CPU_VOID CreateParserTask(CPU_VOID);
#endif