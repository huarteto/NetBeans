/***********************************************************************
 * $Id:: sdk7a404_cf_fat.c 14 2007-08-28 16:08:18Z kevinw              $
 *
 * Project: SDKLH7A404 CF FAT wrapper
 *
 * Description:
 *     This file contains the wrapper for SDKLH7A404 CF FAT.
 *
 ***********************************************************************
 * Software that is described herein is for illustrative purposes only  
 * which provides customers with programming information regarding the  
 * products. This software is supplied "AS IS" without any warranties.  
 * NXP Semiconductors assumes no responsibility or liability for the 
 * use of the software, conveys no license or title under any patent, 
 * copyright, or mask work right to the product. NXP Semiconductors 
 * reserves the right to make changes in the software without 
 * notification. NXP Semiconductors also make no representation or 
 * warranty that such application will be suitable for the specified 
 * use without further testing or modification. 
 **********************************************************************/

#include "lpc_types.h"
#include "sdk7a404_cf_driver.h"
#include "sdk7a404_board.h"


INT_32 cf_cfg;
CF_BLOCKS_T cfb;

INT_32 wrap_init_func(void)
{

    cf_cfg = cf_open((void *)CF_BASE, 0);
	cfb.sector = 1;
	cfb.num_blocks = 1;

	return (1);
}

void wrap_shutdown_func(void)
{
	cf_close(cf_cfg);
	return;
}

STATUS wrap_insert_ck_func(void)
{
	STATUS ret;
	ret = cf_ioctl(cf_cfg, CF_GET_STATUS, CF_CARD_DETECT);
	return (ret);
}

STATUS wrap_ready_ck_func(void)
{
	STATUS ret;
	ret = cf_ioctl(cf_cfg, CF_GET_STATUS, CF_CARD_READY);
	return (ret);
}

STATUS wrap_busy_ck_func(void)
{
	STATUS ret;
	ret = cf_ioctl(cf_cfg, CF_GET_STATUS, CF_CARD_BUSY);
	return (ret);
}

void wrap_set_sector_func(UNS_32 dat)
{
	cfb.sector = dat;
	
	return;
}

void wrap_start_read_func(void)
{
	cf_ioctl(cf_cfg, CF_READ_BLOCKS, (INT_32)&cfb);
	
	return;
}

void wrap_start_write_func(void)
{
	cf_ioctl(cf_cfg, CF_WRITE_BLOCKS, (INT_32)&cfb);
	
	return;
}

INT_32 wrap_read_func(UNS_8 *buf, UNS_32 num)
{
	INT_32 ret;
	
	ret = cf_read(cf_cfg, (void *)buf, num);
	
	return (ret);
}

INT_32 wrap_write_func(UNS_8 *buf, UNS_32 num)
{
	INT_32 ret;
	
	ret = cf_write(cf_cfg, (void *)buf, num);
	
	return (ret);
}

