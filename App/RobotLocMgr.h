#ifndef ROBOTLOCMGR_H
#define ROBOTLOCMGR_H

#include "includes.h"

/*----- f u n c t i o n    p r o t o t y p e s -----*/

CPU_BOOLEAN TestDirection(Position position, CPU_INT08U direction);
void LocationMgrTask(void* data);
void CreateRobotLocMgrTask(void);
CPU_BOOLEAN PositionOccupied(Position position);


#endif