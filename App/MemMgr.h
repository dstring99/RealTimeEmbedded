/*=============== M e m M g r . h ===============*/
#ifndef MEMMGR_H
#define MEMMGR_H

#include "Payload.h"
/*----- c o n s t a n t    d e f i n i t i o n s -----*/

/* Total number of buffers in buffer pool */
#define PoolSize	100

/*----- g l o b a l s -----*/

/* The buffer pool */
extern OS_MEM bfrPool;

/*----- f u n c t i o n    p r o t o t y p e s -----*/

void InitMemMgr(void);
Payload *Allocate(void);
void Free(Payload *bfr);

#endif