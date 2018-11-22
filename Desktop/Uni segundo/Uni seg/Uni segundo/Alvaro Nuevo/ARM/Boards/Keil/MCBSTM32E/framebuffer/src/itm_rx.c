

#include "itm_rx.h"




volatile unsigned int ITM_RxBuffer;        // 0x5AA55AA5 is handshake value




// ITM Serial Window Input Watch and Init function
void ITMRx_InInit(void)
{
  ITM_RxBuffer = 0x5AA55AA5;
}


int ITMRx_Getc(void)
{
  int recChar=0;                                // zero is default if no new char recv
  
  if(ITM_RxBuffer != 0x5AA55AA5)
  {
    recChar = ITM_RxBuffer & 0xff; 
    ITM_RxBuffer = 0x5AA55AA5; 
 
  }
  return(recChar);
}







