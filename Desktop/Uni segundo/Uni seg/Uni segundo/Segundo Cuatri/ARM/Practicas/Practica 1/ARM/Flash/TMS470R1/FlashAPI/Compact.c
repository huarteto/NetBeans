#include "f05.h"
#include "flash470.h"
/********************************************************************
 *                                                                  *
 *  FILENAME: compact.c                                             *
 *  Copyright (c) Texas Instruments 2000-2005, All Rights Reserved  *
 *                                                                  *
 *     Compaction Algorithm                                         *
 *                                                                  *
 *     Returns 1 if compaction completed                            *
 *     Returns 0 if compaction failed                               *
 *                                                                  *
 ********************************************************************/
//#pragma location="API_SEGMENT"
BOOL Flash_Compact_B(UINT32 *start,
		     FLASH_CORE core,
		     FLASH_SECT sector,
		     UINT32 delay,
		     FLASH_ARRAY_ST cntl,
		     FLASH_STATUS_ST *status
		     )
{
  UINT32 save;

  SET_CFG_BIT;
  save=cntl[REGOPT];                   /* save initial mode */
  cntl[REGOPT]=READMODE;               /* force normal read mode */
  init_state_machine(core,delay,cntl); /* initialize */
  *(volatile UINT16 *)start=0x40;      /* clear status register */
  *(volatile UINT16 *)start=0x1000;    /* Validate Sector command */
  *(volatile UINT16 *)start=0xffff;    /* Dummy data value */
  Flash_Track_Pulses_V(cntl,status);   /* track compaction pulses */
  cntl[REGOPT]=save;                   /* restore mode */
  CLR_CFG_BIT;
  /* The below is to preserve backwards compatibility!! */
  status->stat3=status->stat4;         /* Total Compaction Pulses */
  status->stat4=status->stat1;         /* Max Compaction pulses */
  status->stat1=0;                     /* not used */
  return (status->stat2==0);           /* pass if no bits set in MSTAT */;
} /* end of compaction function */
