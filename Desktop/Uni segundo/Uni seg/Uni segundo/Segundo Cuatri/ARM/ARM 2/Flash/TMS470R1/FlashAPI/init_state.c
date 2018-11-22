#include "f05.h"
#include "flash470.h"
/*******************************************************************
 *                                                                 *
 *  FILENAME: init_state.c                                         *
 *  Copyright (c) Texas Instruments 2000-2005, All Rights Reserved *
 *                                                                 *
 *  This function is used to initialize the Flash state machine    *
 *  registers so that the intended timings and pump voltages are   *
 *  used during program/erase/compaction.                          *
 *                                                                 *
 *******************************************************************/
//#pragma location="API_SEGMENT"
void init_state_machine(FLASH_CORE core,
			UINT32 delay,
			FLASH_ARRAY_ST cntl
			)
{
  UINT32 k;

  cntl[MAC2]=(cntl[MAC2]&~0x7)|core;    /* clear bank select bits and set core select */
  cntl[MAC1]|=0x8000;                   /* disable level 1 sector protection */
  cntl[TCR]=0x2fc0;                     /* 0x2fc0 = TCR=0 */
  cntl[MAXPP]=0x0000+MAX_PROG_PLS;      /* start VNV steps at 0, Max programming pulses */
  cntl[MAXCP]=0xf000+MAX_CMPCT_PLS;     /* 16 pulses per VNV step, Max compaction pulses */
  if((cntl[MAXEP]&0xf000)==0xf000) {    /* Baseline 4 material */
    cntl[PTR3]=0x9964;                  /* Set VHV to 9.0v during erase */
    cntl[MAXEP]=0xf000+MAX_ERASE_PLS;   /* max VNV step, Max erase pulses */
  } else {                              /* Baseline 5 or 6 */
    cntl[PTR3]=0x9b64;                  /* Set VHV to 9.0v during erase */
    cntl[MAXEP]=0xa000+MAX_ERASE_PLS;   /* max VNV step, Max erase pulses */
  }
  cntl[PTR4]=0xa000;                    /* Set Vread to 5.0v */
  cntl[PESETUP]=(delay<<4|delay<<8);    /* Set program setup to 500nS, erase setup to 8uS */
  k=(delay|delay<<8);
  cntl[PVEVACCESS]=k;                   /* Program and Erase Verify Access = 500ns */
  k<<=1;
  cntl[PCHOLD]=k;                       /* Program hold and compaction hold =1us */
  cntl[PVEVHOLD]=k;                     /* Program verify hold and erase verify hold =1us */
  cntl[PVEVSETUP]=k;
  cntl[CVACCESS]=delay*16;              /* Compaction verify access time = 8us */
  k=delay*20;
  cntl[CSETUP]=(k|0x3000);              /* Compaction setup = 10us */
  cntl[EHOLD]=(k<<2);                   /* Erase hold = 40us */
  cntl[PWIDTH]=(delay*PROG_PLS_WIDTH);  /* Program width = 4us */
  cntl[CWIDTH]=(delay*CMPCT_PLS_WIDTH); /* Compaction pulse width */
  cntl[EWIDTH]=(delay*ERASE_PLS_WIDTH); /* Erase pulse width */
}
