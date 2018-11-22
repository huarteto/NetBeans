#include "f05.h"
#include "flash470.h"
/********************************************************************
 *                                                                  *
 *  FILENAME: esector.c                                             *
 *  Copyright (c) Texas Instruments 2000-2005, All Rights Reserved  *
 *                                                                  *
 *     Erase Sector Algorithm without pulse tracking                *
 *                                                                  *
 *     Returns 1 if erase completed                                 *
 *     Returns 0 if erase failed                                    *
 *                                                                  *
 ********************************************************************/
//#pragma location="API_SEGMENT"
BOOL Flash_Erase_Sector_B(UINT32 *start,
			  UINT32 length,
			  FLASH_CORE core,
			  FLASH_SECT sector,
			  UINT32 delay,
			  FLASH_ARRAY_ST cntl
			  )
{
  int result=FALSE,k;

  if (delay<=MAX_DELAY) {                /* fail if delay value too high */
    SET_CFG_BIT;
    k=cntl[REGOPT];                      /* save initial mode */
    cntl[REGOPT]=READMODE;               /* force normal read mode */
    init_state_machine(core,delay,cntl);
    Flash_Sector_Select_V(core,cntl);
    *(volatile UINT16 *)start=0x40;      /* clear status register */
    *(volatile UINT16 *)start=0x20;      /* Erase command */
    *(volatile UINT16 *)start=0xffff;
    do {                                 /* while waiting for program to finish */
      Feed_Watchdog_V();                 /* feed watchdog timer */
    } while (cntl[MSTAT]&0x100);         /* until BUSY bit goes low */
    if ((cntl[MSTAT]&0x3ff)==0) {
      result=TRUE;                       /* success if no error bits set */
    }
    cntl[BSEA]=0x0;                      /* Protect all sectors */
    cntl[BSEB]=0x0;                      /* Protect all sectors */
    cntl[REGOPT]=k;                      /* restore mode */
    CLR_CFG_BIT;
  }
  return result;
} /* end of erase function */
