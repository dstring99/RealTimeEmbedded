/*=============== M e m M g r . c ===============*/

#include "includes.h"
#include "assert.h"
#include "Buffer.h"
#include "Globals.h"
#include "MemMgr.h"
#include "Payload.h"

/*----- g l o b a l s -----*/

/* Space for "PoolSize" buffers */
Payload	bfrSpace[PoolSize];

/* Memory partition control block to manage the buffer pool */
OS_MEM bfrPool;

/* Semaphores */
OS_SEM bfrAvail;	/* Number of free buffers remaining in the pool */


/*--------------- I n i t M e m M g r ( ) ---------------*/

/*
PURPOSE
Initialize the memory manager.

GLOBALS
bfrPool		  -- Pool of free buffers
bfrSpace    -- Memory space for pool buffers
bfrAvail -- Semaphore to allow buffer requests when pool is not empty
*/
void InitMemMgr(void)
{
  OS_ERR osErr;/* -- uCos Error Code */   

  /* Create and initialize semaphore. */
  OSSemCreate(&bfrAvail, "Buffer Available", PoolSize, &osErr);
  assert(osErr == OS_ERR_NONE);

  /* Create a pool of "PoolSize" buffers. */
  OSMemCreate(&bfrPool, "Bfr Pool", bfrSpace, PoolSize, sizeof(Payload), &osErr);
  assert(osErr == OS_ERR_NONE);
}

/*--------------- A l l o c a t e ( ) ---------------*/

/*
PURPOSE
Allocate a buffer from the buffer pool - block if none available.

GLOBALS
bfrPool	 -- Pool of free buffers
bfrAvail -- Semaphore to allow buffer requests when pool is not empty

RETURNS
The address of the allocated buffer.
*/
Payload *Allocate(void)
{
  OS_ERR osErr;/* -- uCos Error Code */   

  Payload *bfr;  // Allocated buffer address

  /* Wait until there is an available buffer. */
  OSSemPend(&bfrAvail, 0, OS_OPT_PEND_BLOCKING, NULL, &osErr);
  assert(osErr==OS_ERR_NONE);

  /* Get the buffer from the pool. */
  bfr = (Payload*) OSMemGet(&bfrPool, &osErr);
  assert(osErr==OS_ERR_NONE);
  
  return bfr;
}   	

/*--------------- F r e e ( ) ---------------*/

/*
PURPOSE
Return a buffer to the pool.

INPUT PARAMETERS
bfr   -- the address of the buffer to be returned

GLOBALS
bfrPool	 -- Pool of free buffers
bfrAvail -- Semaphore to allow buffer requests when pool is not empty
*/
void Free(Payload *bfr)
{
  OS_ERR osErr;/* -- uCos Error Code */   

  /* Return the buffer. */
  OSMemPut(&bfrPool, bfr, &osErr);
  assert(osErr==OS_ERR_NONE);

  /* Indicate that one more buffer is available. */
  OSSemPost(&bfrAvail, OS_OPT_POST_1, &osErr);
  assert(osErr==OS_ERR_NONE);
}   