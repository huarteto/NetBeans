#include "f05.h"
#include "flash470.h"
/********************************************************************
 *                                                                  *
 *  FILENAME: sector_select.c                                       *
 *  Copyright (c) Texas Instruments 2000-2005, All Rights Reserved  *
 *                                                                  *
 *     This function enables all sectors in a flash core            *
 *                                                                  *
 ********************************************************************/
//#pragma location="API_SEGMENT"
void Flash_Sector_Select_V(FLASH_CORE core,
			   FLASH_ARRAY_ST cntl
			   )
{
  cntl[BSEA]=0xffff; /* Enable sectors  0-15 */
  cntl[BSEB]=0xffff; /* Enable sectors 16-31 */
}
