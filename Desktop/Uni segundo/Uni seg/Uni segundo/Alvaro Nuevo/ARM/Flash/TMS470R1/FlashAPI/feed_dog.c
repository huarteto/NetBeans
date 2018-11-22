#include "f05.h"
#include "flash470.h"
/*******************************************************************
 *                                                                 *
 *  FILENAME: feed_dog.c                                           *
 *  Copyright (c) Texas Instruments 2000-2005, All Rights Reserved *
 *                                                                 *
 *  This function is used to feed the watchdog timer to prevent    *
 *  a reset                                                        *
 *                                                                 *
 *******************************************************************/
//#pragma location="API_SEGMENT"
void Feed_Watchdog_V()
{
  *(volatile UINT32 *)0xffffff0c=0xe5;
  *(volatile UINT32 *)0xffffff0c=0xa3;
}
