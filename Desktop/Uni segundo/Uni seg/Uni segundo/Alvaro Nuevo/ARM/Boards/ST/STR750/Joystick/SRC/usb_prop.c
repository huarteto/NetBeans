/******************** (C) COPYRIGHT 2006 STMicroelectronics ********************
* File Name          : usb_prop.c
* Author             : MCD Application Team
* Date First Issued  : 03/10/2006 : V0.1
* Description        : All processings related to the Joystick Mouse device
********************************************************************************
* History:
* 03/10/2006 : V0.1
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/

#include "75x_lib.h"

#include "USB_lib.h"
#include "USB_conf.h"
#include "USB_prop.h"
#include "USB_desc.h"
#include "USB_pwr.h"

BYTE *Mouse_GetHidDescriptor(WORD Length);
BYTE *Mouse_GetReportDescriptor(WORD Length);
BYTE *Mouse_GetProtocolValue(WORD Length);
RESULT Mouse_SetProtocol(void);
BYTE ProtocolValue;
				/*Device State for DFU protocol*/

#define USBHPIRQchannel 6
#define USBLPIRQchannel 26

/*------------------------------------------------------------------------------*/
/*------------------------Definition of DeviceState-------------------------*/
/*------------------------------------------------------------------------------*/

BYTE* DeviceState=(BYTE*)(0x2000FFFF);	/*the last Byte in RAM is reserved to DeviceState*/

/* ---------------------------------------------------------------------------- */
/*  Structures initializations */
/* ---------------------------------------------------------------------------- */

DEVICE Device_Table = 
{
/*	ENDP0, */
	EP_NUM,
	1
};

DEVICE_PROP Device_Property = {
	MOUSE_init,
	MOUSE_Reset,
	MOUSE_Status_In,
	MOUSE_Status_Out,
	MOUSE_Data_Setup,
	MOUSE_NoData_Setup,
	MOUSE_Get_Interface_Setting,
	MOUSE_GetDeviceDescriptor,
	MOUSE_GetConfigDescriptor,
	MOUSE_GetStringDescriptor,
	/*MOUSE_EP0Buffer*/0,
	STD_MAXPACKETSIZE
};

/* ---------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
void MOUSE_init()
{
 DEVICE_INFO *pInfo = &Device_Info;

	pInfo->Current_Configuration = 0;
	/* Connect the device */
	PowerOn();
    /* USB interrupts initialization */
    _SetISTR(0);               /* clear pending interrupts */
    wInterrupt_Mask = IMR_MSK;
    _SetCNTR(wInterrupt_Mask); /* set interrupts mask */

        	/*AS Remote Wakeup*/
	if(MOUSE_ConfigDescriptor[7] & 0x20)
		pInfo->Supp_Remote_Wakeup=1;
	else 
		pInfo->Supp_Remote_Wakeup=0;
	pInfo->Current_Feature = 0x40;    /* self-powered, Remote_Wakeup disable*/
        /* Wait until device is configured */
	while (pInfo->Current_Configuration == 0) NOP_Process();

	*DeviceState=0x00;		/*State appIDLE*/

	bDeviceState = CONFIGURED;
} /* MOUSE_init() */


/* ---------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
void MOUSE_Reset()
{
	/* Set MOUSE_DEVICE as not configured */
	Device_Info.Current_Configuration = 0;

	/*correction AS default interface*/
	Device_Info.Current_Interface = 0;/*the default Interface*/
	SetBTABLE(BTABLE_ADDRESS);

	/* Initialize Endpoint 0 */
	SetEPType(ENDP0, EP_CONTROL);
	SetEPTxStatus(ENDP0, EP_TX_STALL/*EP_TX_NAK*/);
	SetEPRxAddr(ENDP0, ENDP0_RXADDR);
	SetEPRxCount(ENDP0, STD_MAXPACKETSIZE);
	SetEPTxAddr(ENDP0, ENDP0_TXADDR);
	SetEPTxCount(ENDP0, STD_MAXPACKETSIZE);
	Clear_Status_Out(ENDP0);
	//SetEPRxStatus(ENDP0, EP_RX_STALL);
	SetEPRxValid(ENDP0);

	/* Initialize Endpoint 1 */
	SetEPType(ENDP1, EP_INTERRUPT);
	SetEPRxStatus(ENDP1, EP_RX_DIS);/* TEST 8.3.3.1.B*/
	SetEPTxStatus(ENDP1, EP_TX_NAK);

	/* Set this device to response on default address */
	SetDeviceAddress(0);

} /* MOUSE_Reset() */

/* ---------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
//#define MOUSE_SetConfiguration	Standard_SetConfiguration
//#define MOUSE_SetInterface()	Standard_SetInterface()

/* ---------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
RESULT MOUSE_Data_Setup(BYTE RequestNo)
{
	DEVICE_INFO		*pInfo = &Device_Info;
	BYTE	*(*CopyRoutine)(WORD);
	BYTE *pbLen;
	WORD wLen;

    CopyRoutine = NULL;
/* correction AS request GET_CONFIGURATION & GET_INTERFACE

	if (RequestNo == GET_CONFIGURATION) {
		if ( Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT) )
			CopyRoutine = Standard_GetConfiguration;
	}
	else
	if (RequestNo == GET_INTERFACE) {
		if (Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT))
			CopyRoutine = Standard_GetInterface;
	}
	else*/
	if (RequestNo == GET_DESCRIPTOR && Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT)) {

		if (pInfo->USBwIndex0 != 0)
			return UNSUPPORT;

		switch (pInfo->USBwValue1) {
			case HID_DESCRIPTOR:
				CopyRoutine = Mouse_GetHidDescriptor;
				break;
			case REPORT_DESCRIPTOR:
				CopyRoutine = Mouse_GetReportDescriptor;
				break;
			default:
				return UNSUPPORT;
		}
	} /* End of GET_DESCRIPTOR */
	else
	/*** GET_IDLE/GET_PROTOCOL ***/
	if (Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT)) {
	   if (RequestNo == GET_IDLE)
	   {
	        // Do something to return the idle time
	        return UNSUPPORT;
	   }
   	   else
	   if (RequestNo == GET_PROTOCOL) {
		CopyRoutine = Mouse_GetProtocolValue;
	   }
	   else return UNSUPPORT;
	}
	else return UNSUPPORT;

	if (CopyRoutine == NULL) return UNSUPPORT;

	pInfo->Ctrl_Info.CopyData = CopyRoutine;
	pInfo->Ctrl_Info.Usb_wOffset = 0;
	pbLen = (*CopyRoutine)(0);
	wLen = (WORD)((DWORD)pbLen);
	pInfo->Ctrl_Info.Usb_wLength = wLen;

	return USB_SUCCESS;

} /* MOUSE_Data_Setup */

/* ---------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
RESULT MOUSE_NoData_Setup(BYTE RequestNo)
{

	DEVICE_INFO		*pInfo = &Device_Info;

	if (Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT)) {
	   if (RequestNo == SET_IDLE)
	   {
	        //Do something to set the idle time
	        return UNSUPPORT;
	   }
   	   else if (RequestNo == DFU_DETACH) {
		return USB_SUCCESS;
	   }
	   else if (RequestNo == SET_PROTOCOL) {
	        return Mouse_SetProtocol();

	   }
       else return UNSUPPORT;
	}

	return UNSUPPORT;

} /* MOUSE_NoData_Setup */

/* ---------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
ONE_DESCRIPTOR Device_Descriptor = {
	(BYTE*)MOUSE_DeviceDescriptor,
	MOUSE_SIZ_DEVICE_DESC
};

BYTE *MOUSE_GetDeviceDescriptor(WORD Length)
{
	return Standard_GetDescriptorData( Length, &Device_Descriptor );
}

ONE_DESCRIPTOR Config_Descriptor = {
	(BYTE*)MOUSE_ConfigDescriptor,
	MOUSE_SIZ_CONFIG_DESC
};

BYTE *MOUSE_GetConfigDescriptor(WORD Length)
{
	return Standard_GetDescriptorData( Length, &Config_Descriptor );
}

ONE_DESCRIPTOR String_Descriptor = {
	(BYTE*)MOUSE_StringDescriptor,
	MOUSE_SIZ_STRING_DESC
};

BYTE *MOUSE_GetStringDescriptor(WORD Length)
{
	return Standard_GetStringDescriptor( Length, &String_Descriptor );
}


ONE_DESCRIPTOR Mouse_Hid_Descriptor = {
	(BYTE *)MOUSE_ConfigDescriptor + MOUSE_OFF_HID_DESC,
	MOUSE_SIZ_HID_DESC
};

BYTE *Mouse_GetHidDescriptor(WORD Length)
{
	return Standard_GetDescriptorData( Length, &Mouse_Hid_Descriptor );
}

ONE_DESCRIPTOR Mouse_Report_Descriptor = {
	(BYTE *)MOUSE_ReportDescriptor,
	MOUSE_SIZ_REPORT_DESC
};

BYTE *Mouse_GetReportDescriptor(WORD Length)
{
	return Standard_GetDescriptorData( Length, &Mouse_Report_Descriptor );
}

BYTE *Mouse_GetProtocolValue(WORD Length)
{
 	return (BYTE *)&ProtocolValue;
}

RESULT Mouse_SetProtocol(void)
{
	BYTE wValue0 = pInformation->USBwValue0;
	ProtocolValue = wValue0;
	return USB_SUCCESS;
}

RESULT MOUSE_Get_Interface_Setting(BYTE Interface,BYTE AlternateSetting)
{
	if(AlternateSetting >0) return UNSUPPORT;// in this application we don't have AlternateSetting
	else if(Interface > 0) return UNSUPPORT;//in this application we have only 1 interfaces
	return USB_SUCCESS;
}

/******************* (C) COPYRIGHT 2006 STMicroelectronics *****END OF FILE****/

