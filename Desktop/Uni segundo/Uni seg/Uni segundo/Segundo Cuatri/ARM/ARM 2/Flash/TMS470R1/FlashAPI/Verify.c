#include "f05.h"
#include "flash470.h"
/********************************************************************
 *                                                                  *
 *  FILENAME: verify.c                                              *
 *  Copyright (c) Texas Instruments 2000-2005, All Rights Reserved  *
 *                                                                  *
 *     Verify Algorithm                                             *
 *                                                                  *
 *     Returns 1 if verify passes                                   *
 *     Returns 0 if verify fails                                    *
 *                                                                  *
 ********************************************************************/
//#pragma location="API_SEGMENT"
BOOL Flash_Verify_B(UINT32 *start,
		    UINT32 *buff,
		    UINT32 length,
		    FLASH_CORE core,
		    FLASH_ARRAY_ST cntl,
		    FLASH_STATUS_ST *status
		    )
{
  volatile UINT32 *dest_addr;            /* Address pointer to flash location */
  UINT32 i,j,k,result=TRUE,sm,sw;
  UBYTE m[3]={READMODE,
	      RDM0MODE,
	      RDM1MODE};                 /* read modes */
  UBYTE w[3]={0x00,0xff,0x11};           /* wait states */

  SET_CFG_BIT;
  sm=cntl[REGOPT];                       /* save current read mode */
  cntl[REGOPT]=READMODE;                 /* force normal read mode */
  cntl[MAC2]=(cntl[MAC2]&~0x7)|core;     /* enable the appropriate core */
  cntl[TCR]=0x2fc0;                      /* 0x2fc0 = TCR=0 */
  cntl[DPTR]=0xb;                        /* clear TEZ */
  sw=cntl[BAC2];                         /* save current wait states */
  cntl[BAC2]=(sw|0xff);                  /* force max wait states */
  for (i=0;(i<3)&&(result==TRUE);++i) {  /* loop through read modes */
    cntl[REGOPT]=m[i];                   /* Set read mode */
    dest_addr=start;                     /* Initialize programmer pointer */
    cntl[BAC2]=(sw&~0xff)|w[i];          /* set wait states */
    for (k=0;k<length;k++) {             /* step through each flash location */
      READ_FLASH_MEM_U32(dest_addr,j);   /* read the data from flash */
      if (j!=buff[k]) break;             /* break if compare fails */
      dest_addr++;                       /* increment address */
    }
    if (k!=length) {
      status->stat1=(UINT32)dest_addr;   /* save last address read */
      status->stat2=j;                   /* save actual data */
      status->stat3=buff[k];             /* save expected data */
      status->stat4=m[i];                /* save read mode */
      result=FALSE;
    }
  }
  cntl[DPTR]=0xf;                        /* reset TEZ */
  cntl[BAC2]=sw;                         /* reset wait states */
  cntl[REGOPT]=sm;                       /* reset read mode */
  CLR_CFG_BIT;
  return result;
} /* end of verify function */
