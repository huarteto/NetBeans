/******************** (C) COPYRIGHT 2003 STMicroelectronics ********************
* File Name          : usb_init.c
* Author             : MCD Application Team
* Date First Issued  : 27/10/2003
* Description        : initialization routines & global variables
*
********************************************************************************/
#include "71x_lib.h"
#include "USB_lib.h"

/*  Save token on endpoint 0 */
 BYTE	EP0_Token  ;

/*  Interrupt flags. */
/*  Each bit represents an interrupt is coming to that endpoint */
 WORD	Token_Event	;

/*  The number of current endpoint, it will be used to specify an endpoint */
 BYTE	EPindex;

/*  The number of current device, it is an index to the Device_Table */
/* BYTE	Device_no; */

/*  Points to the DEVICE_INFO structure of current device */
/*  The purpose of this register is to speed up the execution */
DEVICE_INFO *pInformation;

/*  Points to the DEVICE_PROP structure of current device */
/*  The purpose of this register is to speed up the execution */
DEVICE_PROP *pProperty;

/*  Temporary save the state of Rx & Tx status. */
/*  Whenever the Rx or Tx state is changed, its value is saved */
/*  in this variable first and will be set to the EPRB or EPRA */
/*  at the end of interrupt process */
 WORD	SaveState ;

 WORD  wInterrupt_Mask;

 DEVICE_INFO	Device_Info;

/*==========================================================================*/
/* USB system initialization */
/*==========================================================================*/
void USB_Init()
{
  USB->CNTR_ = 0x0003;
  Token_Event = 0;			/* Flags of each endpoint interrupt */
  pInformation = &Device_Info;
  pInformation->ControlState = 2;
  pProperty = &Device_Property;
  /* Initialize devices one by one */
  pProperty->Init();

} /* USB_Init() */

/*==========================================================================*/
