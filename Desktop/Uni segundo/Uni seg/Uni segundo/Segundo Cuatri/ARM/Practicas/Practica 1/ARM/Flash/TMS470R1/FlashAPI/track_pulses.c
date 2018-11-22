#include "f05.h"
#include "flash470.h"
/********************************************************************
 *                                                                  *
 *  FILENAME: track_pulses.c                                        *
 *  Copyright (c) Texas Instruments 2000-2005, All Rights Reserved  *
 *                                                                  *
 *     Algorithm to track erase pulses in the FSM                   *
 *     This function assumes that an erase or compaction            *
 *     command has been issued and that the config bit is enabled   *
 *                                                                  *
 *     The values in the status structure are updated as follows:   *
 *       status->stat1 => Max Compaction pulses                     *
 *       status->stat2 => final MSTAT value                         *
 *       status->stat3 => Max Erase Pulses                          *
 *       status->stat4 => Total Compaction Pulses                   *
 *                                                                  *
 ********************************************************************/
//#pragma location="API_SEGMENT"
void Flash_Track_Pulses_V(FLASH_ARRAY_ST cntl,
			  FLASH_STATUS_ST *status
			  )
{
  UINT32 k,state,cc=0,ladr=0xFFFF,cadr;

  status->stat1=0;                          /* Max Compaction pulses */
  status->stat2=0;                          /* final MSTAT value */
  status->stat3=0;                          /* Max Erase Pulses */
  status->stat4=0;                          /* Total Compaction Pulses */

  do {
    state=(cntl[WSMREG]&0x7c);              /* retrieve current state */
    if (state==CMPCT_PULSE_ACTIVE) {        /* Compaction pulse */
      status->stat4++;                      /* increment total compaction pulse count */
      cadr=(cntl[ADDRCTRLOW]&0xffff);       /* retrieve lower bits of current address */
      if (cadr!=ladr) {                     /* if new address */
	cc=0;                               /* reset current count */
	ladr=cadr;                          /* update current address */
      }
      if (++cc>=status->stat1) {            /* update current and max pulse counts */
	status->stat1=cc;                   /* update max count */
      }
    } else if (state==ERASE_PULSE_ACTIVE) { /* Erase pulse */
      status->stat3++;                      /* increment max erase pulse count */
    } else {
      state=0xffffffff;                     /* force incorrect state for do loop */
    }
    do {
      Feed_Watchdog_V();                    /* feed watchdog */
    } while ((cntl[WSMREG]&0x7c)==state);   /* while current state matches */
    k=cntl[MSTAT];                          /* retrieve current MSTAT */
  } while (k&0x100);                        /* wait till BUSY bit low */
  status->stat2=(k&0x3ff);                  /* update final MSTAT value */
} /* end of track pulses function */
