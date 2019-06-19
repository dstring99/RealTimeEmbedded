#include "SerIODriver.h"
#include "includes.h"
#include "BfrPair.h"
#include "stm32f10x_map.h"
#include "assert.h"

// If not already defined, use the default buffer size of 4.
#ifndef BfrSize
#define BfrSize 4
#endif

//Define masks to determine if byte are ready to Tx or Rx
#define USART_TXE 0x80
#define USART_RXNE 0x20

//Define Set Enable (interupts) register and mask
#define SETENA1 (*((CPU_INT32U *) 0XE000E104))
#define USART2ENA 0x00000040

//Mask to enable TXE and RXNE in USART_CR1
#define USARTTXERNXE 0x000020AC
#define USARTTXE 0x00000080
#define USARTRXNE 0x00000020

#define SuspendTimeout 10000

// Allocate the input buffer pair.
static BfrPair iBfrPair;
static CPU_INT08U iBfr0Space[BfrSize];
static CPU_INT08U iBfr1Space[BfrSize];

// Allocate the output buffer pair.
BfrPair oBfrPair;
static CPU_INT08U oBfr0Space[BfrSize];
static CPU_INT08U oBfr1Space[BfrSize];

//Declare Bfr Semaphores
static OS_SEM	openOBfrs;
static OS_SEM	closedIBfrs;

// Create and Initialize iBfrPair and oBfrPair.
void InitSerIO(void)
{
  //Initialize input and output buffers to use the above defined bfr spaces
  BfrPairInit(&iBfrPair, iBfr0Space, iBfr1Space, BfrSize);
  BfrPairInit(&oBfrPair, oBfr0Space, oBfr1Space, BfrSize);
  
  //int x = USART2->CR1;
  //Enable Tx and Rx interupts in USART control register
  USART2->CR1 = USARTTXERNXE;
  //x = USART2->CR1;
  
  //Enable interupts by setting appropriate flag in Set Enable register.
  SETENA1 = USART2ENA;
  
  //Create semaphore openOBfrs
  OS_ERR  osErr;  // O/S Error Code
  
  // Create the Open Output Buffer semaphore.
  OSSemCreate(&openOBfrs, "Open OBfrs", 1, &osErr);
  assert(osErr == OS_ERR_NONE);
  
  // Create the Clse Input Buffer semaphore.
  OSSemCreate(&closedIBfrs, "Close IBfrs", 0, &osErr);
  assert(osErr == OS_ERR_NONE);
}
    
// Service the RS232 receiver.
void ServiceRx()
{
  OS_ERR  osErr;  // O/S Error Code
  
  //If byte is not available, return
  if(!(USART2->SR & USART_RXNE))
    return;
  
  //Return if put buffer is closed.
  if(PutBfrIsClosed(&iBfrPair))
  {
    //Disable RXNE
    USART2->CR1 &= ~USARTRXNE;
    
    return;
  }
  //Add byte to put buffer from usart data reg
  PutBfrAddByte(&iBfrPair, USART2->DR);
  
  //Return if put buffer is closed.
  if(PutBfrIsClosed(&iBfrPair))
  {
    /* Signal the Consumer that the buffer is not empty. */
    OSSemPost(&closedIBfrs, OS_OPT_POST_1, &osErr);
    assert(osErr==OS_ERR_NONE);
  }
}

// Service the RS232 transmitter.
void ServiceTx()
{
  OS_ERR  osErr;  // O/S Error Code
  
  //If a byte is not availabe, return
  if(!(USART2->SR & USART_TXE))
    return;
  
  //If get buffer is closed, return
  if(!GetBfrIsClosed(&oBfrPair))
  {
    //Disable TXE
    USART2->CR1 &= ~USARTTXE;
    
    return;
  }
  //Transmit byte from Get buffer
  CPU_INT16S c = GetBfrRemByte(&oBfrPair);
  USART2->DR = c;
  
  //If get buffer is closed, return
  if(GetBfrIsClosed(&oBfrPair))
    return;
  
  /* Signal the Consumer that the buffer is not empty. */
  OSSemPost(&openOBfrs, OS_OPT_POST_1, &osErr);
  assert(osErr==OS_ERR_NONE);
}

CPU_INT16S GetByte()
{
   OS_ERR  osErr;  // O/S Error Code
   CPU_BOOLEAN putBfrNotEmpty;
   
  //If get buffer is empty
  if(!GetBfrIsClosed(&iBfrPair))
  {
    while(1)
    {
      OSSemPend(&closedIBfrs, 20*BfrSize, OS_OPT_PEND_BLOCKING, NULL, &osErr);
      assert(osErr==OS_ERR_NONE || osErr==OS_ERR_TIMEOUT);
      
      //putBfrNotEmpty = !BfrEmpty(&(iBfrPair.buffers[iBfrPair.putBfrNum]));
      putBfrNotEmpty = PutBfrNotEmpty(&iBfrPair);
     
      if((putBfrNotEmpty && osErr==OS_ERR_TIMEOUT) || osErr==OS_ERR_NONE)
      {
        BfrClose(&(iBfrPair.buffers[iBfrPair.putBfrNum]));
        BfrPairSwap(&iBfrPair);
        break;
      }
    }
  }
  
  //Add byte to put buffer.
  CPU_INT16S c = GetBfrRemByte(&iBfrPair);
  
  //Enable TXE
  USART2->CR1 |= USARTRXNE;
  
  return c;
}

CPU_INT16S PutByte(CPU_INT16S txChar)
{
  OS_ERR  osErr;  // O/S Error Code
  
  //If put buffer is still close, return -1
  if(PutBfrIsClosed(&oBfrPair))
  {
    /* Wait for buffer to open */
    OSSemPend(&openOBfrs, 0, OS_OPT_PEND_BLOCKING, NULL, &osErr);
    assert(osErr==OS_ERR_NONE);
    
    BfrPairSwap(&oBfrPair);
  }
  
  //Add byte to put buffer.
  CPU_INT16S c = PutBfrAddByte(&oBfrPair, txChar);
  
  //Enable TXE
  USART2->CR1 |= USARTTXE;
  
  return c;
}

void ForceSend()
{
  OS_ERR  osErr;
  PutBfrClose(&oBfrPair);
  
  OSSemPend(&openOBfrs, 0, OS_OPT_PEND_BLOCKING, NULL, &osErr);
  assert(osErr==OS_ERR_NONE || osErr==OS_ERR_TIMEOUT);
      
  BfrPairSwap(&oBfrPair);
}

void SerialISR(void)
{
  /* Disable interrupts. */
  CPU_SR_ALLOC();
  OS_CRITICAL_ENTER();  
  
  /* Tell kernel we're in an ISR. */
  OSIntEnter();
        
  ServiceRx();
  ServiceTx();

  /* Enable interrupts. */
  OS_CRITICAL_EXIT();
	   
  /* Give the O/S a chance to swap tasks. */
  OSIntExit ();
}