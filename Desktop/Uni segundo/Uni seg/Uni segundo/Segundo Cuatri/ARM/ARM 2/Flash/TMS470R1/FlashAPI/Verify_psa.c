#include "f05.h"
#include "flash470.h"
/********************************************************************
 *                                                                  *
 *  FILENAME: verify_psa.c                                          *
 *  Copyright (c) Texas Instruments 2000-2005, All Rights Reserved  *
 *                                                                  *
 *     PSA Verify Algorithm                                         *
 *                                                                  *
 *     Returns 1 if verify passes                                   *
 *     Returns 0 if verify fails                                    *
 *                                                                  *
 ********************************************************************/
//#pragma location="API_SEGMENT"                                        
BOOL Flash_PSA_Verify_B(UINT32 *start,
			UINT32 length,
			UINT32 psa,
			FLASH_CORE core,
			FLASH_ARRAY_ST cntl,
			FLASH_STATUS_ST *status
			)
{
  UINT32 sm,sw;

  SET_CFG_BIT;
  sm=cntl[REGOPT];                       /* save current read mode */
  cntl[REGOPT]=READMODE;                 /* set READMODE */
  cntl[MAC2]=(cntl[MAC2]&~0x7)|core;     /* enable the appropriate core */
  cntl[TCR]=0x2fc0;                      /* 0x2fc0 = TCR=0 */
  cntl[DPTR]=0xb;                        /* clear TEZ */
  sw=cntl[BAC2];                         /* save current wait states */
  cntl[BAC2]=(sw&~0xff);                 /* set wait states */
  status->stat3=PSA_U32(start,length,0); /* read PSA */
  cntl[REGOPT]=RDM0MODE;                 /* set RDM0MODE */
  cntl[BAC2]=(sw|0xff);                  /* set wait states */
  status->stat1=PSA_U32(start,length,0); /* read PSA */
  cntl[REGOPT]=RDM1MODE;                 /* set RDM1MODE */
  cntl[BAC2]=(sw&~0xff)|0x11;            /* set wait states */
  status->stat2=PSA_U32(start,length,0); /* read PSA */
  cntl[DPTR]=0xf;                        /* reset TEZ */
  cntl[BAC2]=sw;                         /* reset wait states */
  cntl[REGOPT]=sm;                       /* reset read mode */
  CLR_CFG_BIT;

  return ((status->stat1==psa)           /* normal read PSA matches */
	  &&(status->stat2==psa)         /* read margin 0 PSA matches */
	  &&(status->stat3==psa)         /* read margin 1 PSA matches */
	  );
} /* end of psa verify function */
