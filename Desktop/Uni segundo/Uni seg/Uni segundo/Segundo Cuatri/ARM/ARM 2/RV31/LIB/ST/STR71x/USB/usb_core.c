/******************** (C) COPYRIGHT 2003 STMicroelectronics ********************
* File Name          : usb_core.c
* Author             : MCD Application Team
* Date First Issued  : 27/10/2003
* Description        : standard protocol processing (USB v1.1)
*
********************************************************************************/

#include "USB_lib.h"

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

#define ValBit(VAR,Place)    (VAR & (1<<Place))
#define SetBit(VAR,Place)    ( VAR |= (1<<Place) )
#define ClrBit(VAR,Place)    ( VAR &= ((1<<Place)^255) )

BYTE *Standard_GetStatus(WORD Length);
RESULT Standard_ClearFeature(void);
RESULT Standard_SetFeature(void);

#define Send0LengthData() {	\
	_SetEPTxCount(ENDP0, 0);	\
	vSetEPTxStatus(EP_TX_VALID);	\
	}

/* cells saving status during interrupt servicing */
WORD SaveRState;
WORD SaveTState;
#define vSetEPRxStatus(st)	(SaveRState = st)
#define vSetEPTxStatus(st)	(SaveTState = st)

#define USB_StatusIn()	Send0LengthData()
#define USB_StatusOut()	vSetEPRxStatus(EP_RX_VALID)


/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	NAME:	Standard_GetConfiguration
	INPUT:	Length - How many bytes are needed
	OUTPUT:	If "Length" is 0, return the length of configuration value
			If "Length" is not 0, return the data buffer address
	RETURN:	Return 0, if the request is invalid when "Length" is 0
			Return "Buffer" if the "Length" is not 0
	DESCRIPTION:
		Return the current configuration variable address

	NOTICE:
		This routine is used for devices with only one configuration,
		User should write their own routine to process GET_CONFIGURATION
		if their device has more than one configuration.
 *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
BYTE *Standard_GetConfiguration(WORD Length)
{
	if (Length == 0)
		return (BYTE *)sizeof(pInformation->Current_Configuration);

	return (BYTE *)&pInformation->Current_Configuration;
} /* Standard_GetConfiguration */

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	NAME:	Standard_SetConfiguration
	INPUT:
	OUTPUT:
	RETURN:	Return SUCCESS, if the request is performed
			Return UNSUPPORT, if the request is invalid
	DESCRIPTION:
		The class calls this routine to get the configuration value
		Then each class should configure device themself

	NOTICE:
		This routine is used for devices with only one configuration.
		User should write their own routine to process SET_CONFIGURATION
		if their device has more than one configuration.
 *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
RESULT Standard_SetConfiguration(void)
{
	BYTE wValue0 = pInformation->USBwValue0;
	if (wValue0 <= Device_Table.Total_Configuration) {
		/* If the number of configuration is within the range */
		pInformation->Current_Configuration = wValue0;
		return SUCCESS;
	}
	else
		return UNSUPPORT;
} /* Standard_SetConfiguration */

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	NAME:	Standard_GetInterface
	INPUT:	Length - How many bytes are needed
	OUTPUT:	If "Length" is 0, return the length of interface value
			If "Length" is not 0, return the data buffer address
	RETURN:	Return 0, if the request is invalid when "Length" is 0
			Return "Buffer" if the "Length" is not 0
	DESCRIPTION:
		Return the current interface variable address

	NOTICE:
		This routine is used for devices with only one configuration and one interface,
		User should write their own routine to process GET_INTERFACE
		if their device has more than one configuration or one interface
		or one alternative setting.
 *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
BYTE *Standard_GetInterface(WORD Length)
{
	if (Length == 0)
		return (BYTE *)sizeof(pInformation->Current_Interface);

	return (BYTE *)&pInformation->Current_Interface;
} /* Standard_GetInterface */

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	NAME:	Standard_SetInterface
	INPUT:
	OUTPUT:
	RETURN:	Return SUCCESS, if the request is performed
			Return UNSUPPORT, if the request is invalid
	DESCRIPTION:
		The class calls this routine to get the interface value
		Then each class should configure the interface themself

	NOTICE:
		This routine is used for devices with only one configuration and
		one interface with one alternative setting.
		User should write their own routine to process SET_INTERFACE
		if their device has more than one configuration or one interface
		or one alternative setting.
 *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
RESULT Standard_SetInterface(void)
{
	DEVICE_INFO		*pInfo = pInformation;
	if (pInfo->USBwValue0 != 0 || pInfo->USBwIndex0 != 0)
		return UNSUPPORT;

	/* I have only one interface & one alternative setting */
	pInfo->Current_Interface = 0;
	return SUCCESS;
} /* Standard_SetInterface */

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	NAME:	Standard_GetStatus
	INPUT:	Length - How many bytes are needed
	OUTPUT:	If "Length" is 0, return the length of status data
			If "Length" is not 0, return number of bytes have been copied
	RETURN:	Return 0, if the request is at end of data block,
						or is invalid when "Length" is 0
	DESCRIPTION:

	  Copy the device request data to "StatusInfo buffer"
 *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
WORD_BYTE StatusInfo;
#define StatusInfo0 StatusInfo.bw.bb1	/* Reverse bb0 & bb1 */
#define StatusInfo1 StatusInfo.bw.bb0
BYTE *Standard_GetStatus(WORD Length)
{
	DEVICE_INFO	*pInfo = pInformation;
	BYTE	Type_Rec;

	if (Length == 0)
		return (BYTE *)2;

	StatusInfo.w = 0;	/* Reset Status Information */

	Type_Rec = Type_Recipient;
	if (Type_Rec == (STANDARD_REQUEST | DEVICE_RECIPIENT)) {
		BYTE	Feature = pInfo->Current_Feature;

		if (ValBit(Feature, 5))
			SetBit(StatusInfo0, 1); /* Remote Wakeup enabled */

		if (ValBit(Feature, 7))
			;	/*	ClrBit(StatusInfo0, 0); */ /* Bus-powered */
		else if (ValBit(Feature, 6))
			SetBit(StatusInfo0, 0);  /* Self-powered */
	}
	else if (Type_Rec == (STANDARD_REQUEST | ENDPOINT_RECIPIENT)) {
		BYTE	Related_Endpoint;
		BYTE	wIndex0 = pInfo->USBwIndex0;

		Related_Endpoint = ENDP0 + (wIndex0 & 0x0f);

		if (ValBit(wIndex0, 7)) {
			/* IN endpoint */
			if (_GetTxStallStatus( Related_Endpoint ))
				SetBit(StatusInfo0, 0);	/* IN Endpoint stalled */
		}
		else {
			/* OUT endpoint */
			if (_GetRxStallStatus( Related_Endpoint ))
				SetBit(StatusInfo0, 0);	/* OUT Endpoint stalled */
		}

	}
	else
		return NULL;

	return (BYTE *)&StatusInfo;
} /* Standard_GetStatus */

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	NAME:	Standard_ClearFeature
	INPUT:
	OUTPUT:	none
	DESCRIPTION:
		Clear or disable a specific feature
 *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
RESULT Standard_ClearFeature(void)
{
	DEVICE_INFO	*pInfo = pInformation;
	BYTE	Type_Rec = Type_Recipient;
	BYTE	wValue0 = pInfo->USBwValue0;

	if ( Type_Rec == (STANDARD_REQUEST | DEVICE_RECIPIENT) ) {
		if (wValue0 != DEVICE_REMOTE_WAKEUP)
			return UNSUPPORT;

		ClrBit(pInfo->Current_Feature, 5);
		return SUCCESS;
	}
	else if ( Type_Rec == (STANDARD_REQUEST | ENDPOINT_RECIPIENT) ) {
		DEVICE* pDev;
		BYTE	Related_Endpoint;
		BYTE	wIndex0;
		BYTE	rEP;

		if (wValue0 != ENDPOINT_STALL)
			return UNSUPPORT;

		pDev = &Device_Table;
		wIndex0 = pInfo->USBwIndex0;
		rEP = wIndex0 & ~0x80;
		if (rEP >= pDev->Total_Endpoint)
			return UNSUPPORT;

				/* EPindex should be equal to Device_Table.EP0 */
		Related_Endpoint = ENDP0 + rEP;

		if (wIndex0 & 0x80) {		/* IN endpoint */
			if (_GetTxStallStatus(Related_Endpoint ))
				_SetEPTxStatus(Related_Endpoint, EP_TX_NAK);
		}
		else {							/* OUT endpoint */
			if (_GetRxStallStatus(Related_Endpoint)){
				if (Related_Endpoint == ENDP0) {
					/* After clear the STALL, enable the default endpoint receiver */
					SetEPRxCount(Related_Endpoint, STD_MAXPACKETSIZE);
					_SetEPRxStatus(Related_Endpoint, EP_RX_VALID);
				}
				else
					_SetEPRxStatus(Related_Endpoint, EP_RX_NAK);
			}
		}
		return SUCCESS;
	}

	return UNSUPPORT;
} /* Standard_ClearFeature */

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	NAME:	Standard_SetFeature
	INPUT:
	OUTPUT:	none
	DESCRIPTION:
		Set or enable a specific feature
 *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
RESULT Standard_SetFeature(void)
{
	DEVICE_INFO	*pInfo = pInformation;
	BYTE	Type_Rec = Type_Recipient;
	BYTE	wValue0 = pInfo->USBwValue0;

	if ( Type_Rec == (STANDARD_REQUEST | DEVICE_RECIPIENT) ) {
		if (wValue0 != DEVICE_REMOTE_WAKEUP)
			return UNSUPPORT;

		SetBit(pInfo->Current_Feature, 5);

		/****************************************/
		return SUCCESS;
	}
	else if ( Type_Rec == (STANDARD_REQUEST | ENDPOINT_RECIPIENT) ) {
		DEVICE* pDev;
		BYTE	Related_Endpoint;
		BYTE	wIndex0;
		BYTE	rEP;

		if (wValue0 != ENDPOINT_STALL)
			return UNSUPPORT;

		pDev = &Device_Table;
		wIndex0 = pInfo->USBwIndex0;
		rEP = wIndex0 & ~0x80;
		if (rEP >= pDev->Total_Endpoint)
			return UNSUPPORT;

		Related_Endpoint = ENDP0 + rEP;
		if (wIndex0 & 0x80) {		/* IN endpoint */
			_SetEPTxStatus(Related_Endpoint, EP_TX_STALL);
		}
		else {							/* OUT endpoint */
			_SetEPRxStatus(Related_Endpoint, EP_RX_STALL);
		}

		return SUCCESS;
	}

	return UNSUPPORT;
} /* Standard_SetFeature */

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	NAME:	Standard_GetDescriptorData
			Standard_GetStringDescriptor
	INPUT:	Length - Length of the data in this transfer
			pDesc - A pointer points to descriptor struct
					The structure gives the initial address
					of the descriptor and its original size
	OUTPUT:	Address of a part of the descriptor pointed by the Usb_wOffset
			The buffer pointed by this address contains at least Length bytes
	DESCRIPTION:
		These 2 routines only used for the descriptors resident in ROM or RAM
		pDesc can be in either ROM or RAM

		Standard_GetStringDescriptor is used for transfer string descriptor

		The purpose of these 2 routines is to have a versatile way to response
		descriptors request. It allows user to generate certain descriptors
		with software or read descriptors from external storage part by part.
 *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
BYTE *Standard_GetStringDescriptor(WORD Length, ONE_DESCRIPTOR *pDesc)
{
	int		len, offset, wOffset;

	wOffset = pInformation->Ctrl_Info.Usb_wOffset;
	if (Length == 0) {
		offset = 0;
		do {
			len = (int)*(pDesc->Descriptor + offset);
			if (wOffset >= 0 && wOffset < len) {
				len -= wOffset;
				if (len > 0)
					return (BYTE*)len;
				break;
			}

			wOffset -= len;
			offset += len;
		} while (offset < pDesc->Descriptor_Size);

		return 0;
	}

	return pDesc->Descriptor + wOffset;
}/* Standard_GetStringDescriptor */

/*============================================================================*/
/*============================================================================*/
BYTE *Standard_GetDescriptorData(WORD Length, ONE_DESCRIPTOR *pDesc)
{
	int		len, wOffset;

	wOffset = pInformation->Ctrl_Info.Usb_wOffset;
	if (Length == 0) {
		len = pDesc->Descriptor_Size - wOffset;

		if (len <= 0)
			return 0;

		return (BYTE *)len;
	}

	return pDesc->Descriptor + wOffset;
} /* Standard_GetDescriptorData */

/*-----------------------------------------------------------------------------
ROUTINE NAME : DataStageOut
INPUT/OUTPUT : None
DESCRIPTION  : Data stage of a Control Write Transfer
-----------------------------------------------------------------------------*/
void DataStageOut()
{
	ENDPOINT_INFO	*pEPinfo = &pInformation->Ctrl_Info;
	WORD	save_rLength;

	save_rLength = pEPinfo->Usb_rLength;

	if (pEPinfo->CopyData && save_rLength) {
		BYTE *Buffer;
		WORD Length;
		BYTE *Source;

		Length = pEPinfo->PacketSize;
		if (Length > save_rLength)
			Length = save_rLength;

		Buffer = (*pEPinfo->CopyData)(Length);

		pEPinfo->Usb_rLength -= Length;
		pEPinfo->Usb_rOffset += Length;

/*sb		Source = pProperty->RxEP_buffer; */
		Source = PMAAddr + (BYTE *)(_GetEPRxAddr(ENDP0)*2); /* *2 for 32 bits addr */
		while (Length) {
			*Buffer++ = *Source++;
			Length--;
			if(Length == 0) break; /* odd counter */
			*Buffer++ = *Source++;
			Length--;
			Source++;Source++; /* skip 2 bytes for 32 bit addressing */
		}
	}

	if (pEPinfo->Usb_rLength == 0) {
		/* this OUT Transaction is the last one */
		pInformation->ControlState =
			(save_rLength == pEPinfo->PacketSize) ?
					/*	If the previous read length is same as MaxPacketSize
						This is the multiple MaxPacketSize package
						Wait for another OUT token and send 0 length data
					*/
				WAIT_OUT_ZERO :
					/*	If the previous read length is NOT same as MaxPacketSize
						Wait for the IN token to finish the status stage
					*/
				LAST_OUT_DATA;
	}
} /* DataStageOut */

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	NAME:	DataStageIn
	INPUT:	none
	OUTPUT:
	DESCRIPTION:
		Data stage of a Control Read Transfer
 *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
void DataStageIn(void)
{
	ENDPOINT_INFO	*pEPinfo = &pInformation->Ctrl_Info;
	WORD	save_wLength = pEPinfo->Usb_wLength;
	BYTE	ControlState;

	BYTE	*DataBuffer;
	WORD	Length;
	int i;
    DWORD *pTxBuff;
    WORD wTra;
 	union {
		BYTE *bTra;
    	WORD *wTra;
	}pBuf;

	if (save_wLength == 0) {
		/* if the number of byte to be sent is				*/
		/* multiple of MaxPacketSize: send a 0 length data packet	*/
		ControlState = WAIT_IN_ZERO;
		Send0LengthData();
		goto Expect_Status_Out;
	}

	Length = pEPinfo->PacketSize;
	ControlState = (save_wLength < Length) ? LAST_IN_DATA : IN_DATA;

	/* Same as UsbWrite */
	if (Length > save_wLength)
		Length = save_wLength;

	DataBuffer = (*pEPinfo->CopyData)(Length);
    /* transfer data from buffer to PMA */
    pTxBuff = (DWORD *)(PMAAddr + (BYTE *)(_GetEPTxAddr(ENDP0)*2));
    pBuf.wTra = &wTra;
    for(i=0;i < Length;)
    {
    	*(pBuf.bTra  ) = *DataBuffer++;
    	i++;
    	*(pBuf.bTra+1) = *DataBuffer++;
    	i++;
    	*pTxBuff = wTra;
    	pTxBuff++;
    }

	_SetEPTxCount(ENDP0, Length);

	pEPinfo->Usb_wLength -= Length;
	pEPinfo->Usb_wOffset += Length;
	vSetEPTxStatus(EP_TX_VALID);

Expect_Status_Out:
	USB_StatusOut();	/* Expect the host to abort the data IN stage */
	pInformation->ControlState = ControlState;
}/* DataStageIn */

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	NAME:	NoData_Setup0
	INPUT:
	OUTPUT:	none
	DESCRIPTION:
		Proceed the processing of setup request without data stage
 *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
void NoData_Setup0()
{
	DEVICE_INFO	*pInfo = pInformation;
	RESULT	Result;
	BYTE	RequestNo = pInfo->USBbRequest;
	BYTE	ControlState;

	if (RequestNo == CLEAR_FEATURE)
		Result = Standard_ClearFeature();
	else if (RequestNo == SET_FEATURE)
		Result = Standard_SetFeature();
	else if (RequestNo == SET_ADDRESS && Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT) )
			Result = SUCCESS;
	else
		Result = UNSUPPORT;

	if (Result != SUCCESS) {
		Result = (*pProperty->Class_NoData_Setup)(RequestNo);
		if (Result == NOT_READY) {
			ControlState = PAUSE;
			goto exit_NoData_Setup0;
		}
	}

	if (Result != SUCCESS) {
		ControlState = STALLED;
		goto exit_NoData_Setup0;
	}

	ControlState = WAIT_STATUS_IN;	/* After no data stage SETUP */
	USB_StatusIn();

exit_NoData_Setup0:
	pInformation->ControlState = ControlState;
	return;
} /* NoData_Setup0 */


/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	NAME:	Data_Setup0
	INPUT:
	OUTPUT:	none
	DESCRIPTION:
		Proceed the processing of setup request with data stage
 *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
void Data_Setup0()
{
	DEVICE_INFO		*pInfo = pInformation;
	DEVICE_PROP		*pProp = pProperty;
	BYTE	*(*CopyRoutine)(WORD);
	WORD	wOffset;
	RESULT	Result;
	BYTE Request_No = pInfo->USBbRequest;
BYTE *pbLen;
WORD wLen;

	CopyRoutine = NULL;
	wOffset = 0;

	if (Request_No == GET_DESCRIPTOR) {
		if (Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT)) {
			BYTE wValue1 = pInfo->USBwValue1;
			if (wValue1 == DEVICE_DESCRIPTOR)
				CopyRoutine = pProp->GetDeviceDescriptor;
			else if (wValue1 == CONFIG_DESCRIPTOR)
				CopyRoutine = pProp->GetConfigDescriptor;
			else if (wValue1 == STRING_DESCRIPTOR) {
				wOffset = pInfo->USBwValue0;
				CopyRoutine = pProp->GetStringDescriptor;
			}		/* End of GET_DESCRIPTOR */
		}
	}
	else if (Request_No == GET_STATUS) {
		if (Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT)
				|| Type_Recipient == (STANDARD_REQUEST | ENDPOINT_RECIPIENT)) {
			CopyRoutine = Standard_GetStatus;
		}
	}
/***************************************************************************
	else if (Request_No == GET_CONFIGURATION) {
		if ( Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT) )
			CopyRoutine = Standard_GetConfiguration;
	}
	else if (Request_No == GET_INTERFACE) {
		if (Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT))
			CopyRoutine = Standard_GetInterface;
	}
	else if (Request_No == SET_DESCRIPTOR:) {
		if (Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT)) {
			BYTE *SetDeviceDescriptor(WORD Length);
			BYTE wValue1 = pInfo->USBwValue1;
			if ( wValue1 == DEVICE_DESCRIPTOR)
				CopyRoutine = SetDeviceDescriptor;
			else if (wValue1 == CONFIG_DESCRIPTOR)
				CopyRoutine = SetConfigDescriptor;
			else if (wValue1 == STRING_DESCRIPTOR) {
				CopyRoutine = SetStringDescriptor;
				wOffset = pInfo->USBwValue0;
			}
		}
	}
***************************************************************************/

	if (CopyRoutine) {
		pInfo->Ctrl_Info.Usb_wOffset = wOffset;
		pInfo->Ctrl_Info.CopyData = CopyRoutine;
		/* sb in the original the cast to word was directly */
		/* now the cast is made step by step */
		pbLen = (*CopyRoutine)(0);
		wLen = (WORD)((DWORD)pbLen);
		pInfo->Ctrl_Info.Usb_wLength = wLen;
		Result = SUCCESS;
	}
	else {
		Result = (*pProp->Class_Data_Setup)(pInfo->USBbRequest);
		if (Result == NOT_READY) {
			pInfo->ControlState = PAUSE;
			return;
		}
	}

	if (pInfo->Ctrl_Info.Usb_wLength == -1) {	/* Data is not ready, wait it */
		pInfo->ControlState = PAUSE;
		return;
	}
	if (Result == UNSUPPORT || pInfo->Ctrl_Info.Usb_wLength == 0) {
		/* Unsupported request */
		pInfo->ControlState = STALLED;
		return;
	}

	if (ValBit(pInfo->USBbmRequestType, 7)) {
		/* Device ==> Host */
		WORD	wLength = pInfo->USBwLength;

		/* Restrict the data length to be the one host asks */
		if (pInfo->Ctrl_Info.Usb_wLength > wLength)
			pInfo->Ctrl_Info.Usb_wLength = wLength;

		pInfo->Ctrl_Info.PacketSize = pProp->MaxPacketSize;
		DataStageIn();
	}
	else {
		pInfo->ControlState = OUT_DATA;
/*		SetEPRxCount(EPindex, STD_MAXPACKETSIZE);	*/
		vSetEPRxStatus(EP_RX_VALID);
								/* reenable for next data reception */
	}

	return;
} /* Data_Setup0 */

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	NAME:	Setup0_Process
	INPUT:
	OUTPUT:	none
	DESCRIPTION:
		Get the device request data and dispatch to individual process
 *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
BYTE Setup0_Process()
{
	DEVICE_INFO	*pInfo = pInformation;
	union {
		BYTE*	b;
		WORD*	w;
	} pBuf;
/* sb	pBuf.b = pProperty->RxEP_buffer; */
		pBuf.b = PMAAddr + (BYTE *)(_GetEPRxAddr(ENDP0)*2); /* *2 for 32 bits addr */

	if (pInfo->ControlState != PAUSE) {
		pInfo->USBbmRequestType = *pBuf.b++;	/* bmRequestType */
		pInfo->USBbRequest		= *pBuf.b++;	/* bRequest */
		pBuf.w++;  /* sb word not accessed because of 32 bits addressing */
		pInfo->USBwValue		= ByteSwap(*pBuf.w++);	/* wValue */
		pBuf.w++;  /* word not accessed because of 32 bits addressing */
		pInfo->USBwIndex		= ByteSwap(*pBuf.w++);	/* wIndex */
		pBuf.w++;  /* word not accessed because of 32 bits addressing */
/* sb		pInfo->USBwLength		= ByteSwap(*pBuf.w);*/	/* wLength */
		pInfo->USBwLength		= *pBuf.w;	/* wLength */
	}

	pInfo->ControlState	= SETTING_UP;
	if (pInfo->USBwLength == 0)
	{	/* No data statge processing */
		NoData_Setup0();
	}
	else {			/* Setup with data stage */
		Data_Setup0();
	}
	return Post0_Process();
} /* Setup0_Process */

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	NAME:	In0_Process
	INPUT:
	OUTPUT:	none
	DESCRIPTION:
		Process the IN token on all default endpoint
 *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
BYTE In0_Process()
{
	DEVICE_INFO	*pInfo = pInformation;
	BYTE	ControlState = pInfo->ControlState;

	if (ControlState == IN_DATA)
	{
		DataStageIn();
/* sb questo e' un baco della libreria st9	*/
		ControlState = pInfo->ControlState; /* may be changed outside the function */
/* sb */
	}
	else if (ControlState == LAST_IN_DATA || ControlState == WAIT_IN_ZERO) {
		ControlState = WAIT_STATUS_OUT;
		USB_StatusOut();
	}
	else if (ControlState == WAIT_OUT_ZERO || ControlState == WAIT_STATUS_IN) {
		if (pInfo->USBbRequest == SET_ADDRESS &&
				Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT) ) {

			/* Device address must be written */
			/* after completion of Status Stage (ACK from Host) */
			SetDeviceAddress(pInfo->USBwValue0);

		}

		(*pProperty->Process_Status_IN)();

		ControlState = WAIT_SETUP;
	}
	else
		ControlState = STALLED;

	pInfo->ControlState = ControlState;

	return Post0_Process();
} /* In0_Process */

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	NAME:	Out0_Process
	INPUT:
	OUTPUT:	none
	DESCRIPTION:
		Process the OUT token on all default endpoint
 *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
BYTE Out0_Process()
{
	DEVICE_INFO	*pInfo = pInformation;
	BYTE	ControlState = pInfo->ControlState;

	if (ControlState == OUT_DATA) {
		DataStageOut();

		if (pInfo->ControlState == LAST_OUT_DATA) {
			ControlState = WAIT_STATUS_IN;
			USB_StatusIn();
		}
		else {
			/* Expecting another OUT token with 0 length data */
			SetEPRxCount(ENDP0, 0);
			vSetEPRxStatus(EP_RX_VALID);

			/* Also expecting an IN token to finish the transaction */
			USB_StatusIn();
		}
	}
	else if (ControlState == WAIT_STATUS_OUT || ControlState == IN_DATA) {
		/* host aborts the xfer before finish */
		/* Clear_Status_Out(EPindex);*/		/* Clear ST_OUT bit of this EP */

		vSetEPTxStatus(EP_TX_NAK);
				/* This is to ensure that when the xfer is aborted,
					close down the transmitter, in case the next IN
					token comes in before I config the transmitter */

		(*pProperty->Process_Status_OUT)();
		ControlState = WAIT_SETUP;
	}
	else if (ControlState == WAIT_OUT_ZERO) {
		ControlState = WAIT_STATUS_IN;
		USB_StatusIn();

	}
	else {
		/* Unexpect state, STALL the endpoint */
		ControlState = STALLED;
	}

	pInfo->ControlState = ControlState;

	return Post0_Process();
} /* Out0_Process */

/*============================================================================*/
/*============================================================================*/
BYTE Post0_Process()
{
	SetEPRxCount(ENDP0, STD_MAXPACKETSIZE);

	if (pInformation->ControlState == STALLED) {
		vSetEPRxStatus(EP_RX_STALL);
		vSetEPTxStatus(EP_TX_STALL);
	}
/*
 * Since the SIE will receive SETUP even in NAK state
 * There is no need to enable the receiver again
*/

	return (pInformation->ControlState == PAUSE);
} /* Post0_Process */

/***************************************************************************
BYTE DescriptorBuffer[128];
BYTE *SetDeviceDescriptor(WORD Length)
{
	if (Length == 0)
		return (BYTE*)sizeof(DescriptorBuffer);

	return DescriptorBuffer + pInfomation->Ctrl_Info.Usb_rOffset;
}
***************************************************************************/
/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
void SetDeviceAddress(BYTE Val)
{
  int i;
  DEVICE *pDevice = &Device_Table;
/*  BYTE	EP0 = pDevice->EP0;	*/
  int	nEP = pDevice->Total_Endpoint;

	/* set address in every used endpoint */
	for(i=0;i<nEP;i++)
	{
		_SetEPAddress((BYTE)i, (BYTE)i);
	} /* for */
	_SetDADDR(Val|DADDR_EF); /* set device address and enable function */
} /* SetDeviceAddress */

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
void NOP_Process(void)
{
}

