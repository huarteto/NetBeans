#include "f05.h"
#include "flash470.h"
/********************************************************************
 *                                                                  *
 *  FILENAME: prog.c                                                *
 *  Copyright (c) Texas Instruments 2000-2005, All Rights Reserved  *
 *                                                                  *
 *     Programming Algorithm                                        *
 *                                                                  *
 *     Returns 1 if program completed                               *
 *     Returns 0 if program failed                                  *
 *                                                                  *
 ********************************************************************/
//#pragma location="API_SEGMENT"
BOOL Flash_Prog_B(UINT32 *start,
		  UINT32 *buff,
		  UINT32 length,
		  FLASH_CORE core,
		  UINT32 delay,
		  FLASH_ARRAY_ST cntl,
		  FLASH_STATUS_ST *status
		  )
{
  volatile UINT16 *cadr=(UINT16 *)start;
  UINT16 retries=0,maxpp;
  UINT32 i,j,k,mstat,result=TRUE;

  status->stat3=0;                      /* initialize total number of pulses */
  status->stat4=0;                      /* initialize max pulses */
  length<<=1;                           /* double word length due to 16 bit programming */

  SET_CFG_BIT;
  init_state_machine(core,delay,cntl);  /* initialize state machine */
  i=cntl[BAC2];                         /* save current wait states */
  cntl[BAC2]=(i|0xff);                  /* force max wait states */
  j=cntl[REGOPT];                       /* save initial mode */
  cntl[REGOPT]=READMODE;                /* force normal read mode */
  Flash_Sector_Select_V(core,cntl);     /* select core and enable sectors */
  maxpp=cntl[MAXPP];                    /* retrieve max allowed programming pulses */
  for (k=0;k<length;k++) {              /* Step through each flash location */
    *cadr=0x40;                         /* clear status register */
    *cadr=0x10;                         /* Program command */
    *cadr++=*((UINT16 *)buff+k);        /* data to be programmed */
    do {                                /* while waiting for program to finish */
      Feed_Watchdog_V();                /* feed watchdog timer */
      mstat=cntl[MSTAT];                /* retrieve MSTAT */
    } while (mstat&0x100);              /* until BUSY bit goes low */
    mstat&=0x3ff;                       /* keep only bits of interest */
    retries=maxpp-cntl[PULSE_COUNTER];  /* calculate retries */
    if (retries>status->stat4) {        /* if latest count is max */
      status->stat4=retries;            /* update max pulses */
    }
    status->stat3+=retries;             /* update total pulses */
    if ((retries==maxpp)                /* if max pulses reached */
	||(mstat!=0)                    /* or MSTAT non-zero */
	) {
      status->stat1=(UINT32)(--cadr);   /* retrieve last 32 bit address */
      status->stat2=*cadr;              /* retrieve last data */
      status->stat3=(mstat<<1);         /* retrieve last MSTAT value */
      result=FALSE;                     /* set fail flag */
      k=length;                         /* exit loop */
    }
  } /* end of for */
  cntl[BSEA]=0x0;                       /* Protect all sectors */
  cntl[BSEB]=0x0;                       /* Protect all sectors */
  cntl[BAC2]=i;                         /* restore wait states */
  cntl[REGOPT]=j;                       /* restore mode */
  CLR_CFG_BIT;
  status->stat3>>=1;                    /* div by 2 since prog 16-bits at a time */
  return result;
} /* end of program function */
