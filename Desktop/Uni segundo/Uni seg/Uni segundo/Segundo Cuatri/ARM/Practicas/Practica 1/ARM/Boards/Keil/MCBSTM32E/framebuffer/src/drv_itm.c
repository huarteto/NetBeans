
#include <stm32f10x.h>
#include <RTL.h>                      /* RTX kernel functions & defines      */
#include "drv_itm.h"
#include "itm_rx.h"





// ---- ITM  ----
int  Open_ITM     (void) 
{
  ITMRx_InInit();
 
  return(0);
}

int  Close_ITM    (void) 
{
  return(0); 
}

unsigned int  Input_ITM    (unsigned char *buf, unsigned int len)
{
  buf[0] = 0;
  
  if(len--)
	{
		while(!buf[0])
    {
      buf[0] = ITMRx_Getc();
      os_dly_wait(2);
    }
	}
  return(len);
}

unsigned int  Output_ITM   (const unsigned char *buf, unsigned int len)
{
  for (; len; len--) 
  {
  	itm_putchar_retarget(*buf++);
  }
  return(0);
}


// ----------------------   driver   -------------------------------
__inline int itm_putchar_retarget(int c)
{
 	if (c == '\n')
	{
		ITM_SendChar('\r');
	}		
	ITM_SendChar(c);

	return(c);
}

