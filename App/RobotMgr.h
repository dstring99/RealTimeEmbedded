#ifndef ROBOTMGR_H
#define ROBOTMGR_H

/*----- f u n c t i o n    p r o t o t y p e s -----*/

void CreateRobotMgrTask(void);

//Semaphores to lock and release dir test
extern OS_SEM openDirTest;
extern CPU_BOOLEAN locationMatrix[POS_XMAX+1][POS_YMAX+1];
extern CPU_INT08S offsetMatrix[9][2];

#endif