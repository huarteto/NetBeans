#include "f05.h"
#include "flash470.h"
/********************************************************************
 *                                                                  *
 *  FILENAME: erase.c                                               *
 *  Copyright (c) Texas instruments 2000-2005, All Rights Reserved  *
 *                                                                  *
 *     Erase Algorithm with or w/o Precondition step                *
 *                                                                  *
 *     If status->stat1==0x12345678, precondition not performed     *
 *     Returns 1 if erase completed                                 *
 *     Returns 0 if erase failed                                    *
 *                                                                  *
 ********************************************************************/
//#pragma location="API_SEGMENT"
BOOL Flash_Erase_B(UINT32 *start,
		   UINT32 length,
		   FLASH_CORE core,
		   FLASH_SECT sector,
		   UINT32 delay,
		   FLASH_ARRAY_ST cntl,
		   FLASH_STATUS_ST *status
		   )
{
  UINT32 j,k,noprcnd=status->stat1;     /* noprcnd = no precondition flag */

  if (delay>MAX_DELAY) {
    status->stat2=0xff;                   /* Error - delay value too high */
  } else {
    SET_CFG_BIT;
    k=cntl[REGOPT];                       /* save initial mode */
    cntl[REGOPT]=READMODE;                /* force normal read mode */
    init_state_machine(core,delay,cntl);  /* initialize for erase */
    Flash_Sector_Select_V(core,cntl);
    if (noprcnd==0x12345678) {
      j=cntl[MAXPP];                      /* save original MAXPP for later restore */
      cntl[MAXPP]=(j&0xf000);             /* force zero programming pulses */
    }
    *(volatile UINT16 *)start=0x40;       /* clear status register */
    *(volatile UINT16 *)start=0x20;       /* Erase command */
    *(volatile UINT16 *)start=0xffff;     /* Dummy data value */
    Flash_Track_Pulses_V(cntl,status);    /* track erase and compaction pulses */
    cntl[BSEA]=0x0;                       /* Protect all sectors */
    cntl[BSEB]=0x0;                       /* Protect all sectors */
    if (noprcnd==0x12345678) {
      cntl[MAXPP]=j;                      /* restore original MAXPP */
    }
    cntl[REGOPT]=k;                       /* restore mode */
    CLR_CFG_BIT;
  }
  return (status->stat2==0);              /* pass if no bits set in MSTAT */
} /* end of erase function */
