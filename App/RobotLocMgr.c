#include "includes.h"
#include "Globals.h"
#include "RobotLocMgr.h"
#include "assert.h"
#include "os.h"

//Robot location matrix
CPU_BOOLEAN locationMatrix[POS_XMAX+1][POS_YMAX+1] = {FALSE};

typedef struct {
  Position position;
  CPU_INT08U direction;
  CPU_BOOLEAN result;
} DirTestStruct;

//Direction offsets, used to test direction. See globas for def.
CPU_INT08S offsetMatrix[9][2] = {
  OFF_DIR_0,
  OFF_DIR_N,
  OFF_DIR_NE,
  OFF_DIR_E,
  OFF_DIR_SE,
  OFF_DIR_S,
  OFF_DIR_SW,
  OFF_DIR_W,
  OFF_DIR_NW
};

//Test location for occupation
CPU_BOOLEAN TestDirection(Position position, CPU_INT08U direction)
{
  CPU_INT08U x = position.x + offsetMatrix[direction][0];
  CPU_INT08U y = position.y + offsetMatrix[direction][1];
  return locationMatrix[x][y];
}

CPU_BOOLEAN PositionOccupied(Position position)
{
  return locationMatrix[position.x][position.y];
}