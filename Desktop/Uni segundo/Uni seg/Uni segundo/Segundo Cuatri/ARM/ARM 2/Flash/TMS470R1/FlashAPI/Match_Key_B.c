#include "f05.h"
#include "flash470.h"
/*******************************************************************
 *                                                                 *
 *  FILENAME: Match_Key_B.c                                        *
 *  Copyright (c) Texas Instruments 2000-2005, All Rights Reserved *
 *                                                                 *
 *  This function is used to match the key values in the           *
 *  protection keys before an erase/program/read of the Flash      *
 *  can be performed.  It returns true if the keys read out        *
 *  match the expected keys, in which case the match was           *
 *  successful                                                     *
 *                                                                 *
 *******************************************************************/
//#pragma location="API_SEGMENT"
BOOL Flash_Match_Key_B(volatile UINT32 *key_start,
		       const UINT32 key[],
		       FLASH_ARRAY_ST cntl
		       )
{
  int k,sm,sw;

  SET_CFG_BIT;
  k=cntl[MSTAT];                      /* retrieve MSTAT value */
  if (!(k&0x8)) {                     /* if 3VSTAT bit not set */
    cntl[MAC2]=(cntl[MAC2]&~0x7);     /* select bank 0 */
    do {
      Feed_Watchdog_V();              /* feed the dog */
    } while (!(cntl[BPTR]&0x200));    /* while waiting for pump ready */
    sw=cntl[BAC2];                    /* save current wait states */
    cntl[BAC2]=(sw|0xff);             /* force max wait states */
    sm=cntl[REGOPT];                  /* save current mode */
    cntl[REGOPT]=READMODE;            /* force normal read mode */
    for (k=0;k<4;k++) {
      key_start[k];
      cntl[PROTKEY]=key[k];
    }
    k=(cntl[BBUSY]&0x8000)>>15;       /* retrieve result */
    cntl[REGOPT]=sm;                  /* restore read mode */
    cntl[BAC2]=sw;                    /* restore wait states */
  }
  CLR_CFG_BIT;
  return k;                           /* return value */
}
