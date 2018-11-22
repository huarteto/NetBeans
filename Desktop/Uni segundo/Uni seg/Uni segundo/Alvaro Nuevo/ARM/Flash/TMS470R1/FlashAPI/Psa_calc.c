#include "f05.h"
#include "flash470.h"
/********************************************************************
 *                                                                  *
 *  FILENAME: psa_calc.c                                            *
 *  Copyright (c) Texas Instruments 2000-2005, All Rights Reserved  *
 *                                                                  *
 *     PSA Calculation Algorithm                                    *
 *                                                                  *
 *     Generates a 32 bit PSA for the given region of Flash         *
 *                                                                  *
 ********************************************************************/
//#pragma location="API_SEGMENT"
UINT32 Flash_PSA_Calc_U32(UINT32 *start,
			  UINT32 length,
			  UINT32 psa_seed,
			  UINT32 mode,
			  FLASH_CORE core,
			  FLASH_ARRAY_ST cntl
			  )
{
  UINT32 sm,sw,psa;

  SET_CFG_BIT;
  sm=cntl[REGOPT];                    /* save initial mode */
  if (mode==1) {
    cntl[REGOPT]=RDM0MODE;            /* set read margin 0 mode */
  } else if (mode==2) {
    cntl[REGOPT]=RDM1MODE;            /* set read margin 1 mode */
  } else {
    cntl[REGOPT]=READMODE;            /* set normal read mode */
  }
  cntl[MAC2]=(cntl[MAC2]&~0x7)|core;  /* enable the appropriate core */
  cntl[TCR]=0x2fc0;                   /* 0x2fc0 = TCR=0 */
  cntl[DPTR]=0xb;                     /* clear TEZ */
  sw=cntl[BAC2];                      /* save current wait states */
  cntl[BAC2]=(sw|0xff);               /* set wait states */
  psa=PSA_U32(start,length,psa_seed); /* read to generate PSA */
  cntl[DPTR]=0xf;                     /* reset TEZ */
  cntl[BAC2]=sw;                      /* reset wait states */
  cntl[REGOPT]=sm;                    /* reset read mode */
  CLR_CFG_BIT;
  return psa;
} /* end of psa calc function */
