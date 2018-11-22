/******************** (C) COPYRIGHT 2006 STMicroelectronics ********************
* File Name          : usb_core.c
* Author             : MCD Application Team
* Date First Issued  : 03/10/2006 : V0.1
* Description        : Standard protocol processing (USB v2.0)
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

#include "USB_lib.h"

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

#define ValBit(VAR,Place)    (VAR & (1<<Place))
#define SetBit(VAR,Place)    ( VAR |= (1<<Place) )
#define ClrBit(VAR,Place)    ( VAR &= ((1<<Place)^255) )

BYTE *Standard_GetStatus(WORD Length);
RESULT Standard_ClearFeature(void);
extern BYTE* test;


#define Send0LengthData() {	\
	_SetEPTxCount(ENDP0, 0);	\
	vSetEPTxStatus(EP_TX_VALID);	\
	}

/* cells saving status during interrupt servicing */
WORD SaveRState;
WORD SaveTState;
BYTE EXTRA=0;
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
	RETURN:	Return USB_SUCCESS, if the request is performed
			Return UNSUPPORT, if the request is invalid
	DESCRIPTION:
		This routine is called to set the configuration value
		Then each class should configure device themself

	NOTICE: this routine can support 127 configurations 	
 *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

RESULT Standard_SetConfiguration(void)
{
	if (pInformation->USBwValue0 <= Device_Table.Total_Configuration && pInformation->USBwValue1==0
	    && pInformation->USBwIndex==0) //call Back usb spec 2.0
	{
		
		pInformation->Current_Configuration = pInformation->USBwValue0;
		return USB_SUCCESS;
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
		Return the Alternate Setting of the current interface

	NOTICE: Every Interface can Support 128 Alternate Settings
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

BYTE *Standard_GetInterface(WORD Length)
{
	if (Length == 0)
		return (BYTE *)sizeof(pInformation->Current_AlternateSetting);

	return (BYTE *)&pInformation->Current_AlternateSetting;
} /* Standard_GetInterface */

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	NAME:	Standard_SetInterface
	INPUT:
	OUTPUT:
	RETURN:	Return USB_SUCCESS, if the request is performed
			Return UNSUPPORT, if the request is invalid
	DESCRIPTION:
		This routine is called to set the interface
		Then each class should configure the interface them self

	NOTICE: The number of supported interfaces should be specified by the user.
		The maximum number of supported Interface is 128
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
RESULT Standard_SetInterface(void)
{
	DEVICE_INFO		*pInfo = pInformation;
	DEVICE_PROP		*pProp = pProperty;
	RESULT                  Re;
	
	//test if the specified Interface and Alternate Setting are supportted by the application Firmware
	Re = (*pProp->Class_Get_Interface_Setting)(pInfo->USBwIndex0,pInfo->USBwValue0);

	if(pInfo->Current_Configuration==0 )
			return UNSUPPORT;
	else
	{
		if (Re!= USB_SUCCESS || pInfo->USBwIndex1!=0 || pInfo->USBwValue1!=0)
				return  UNSUPPORT;
		else if ( Re == USB_SUCCESS)
		{
				pInfo->Current_Interface = pInfo->USBwIndex0;
				pInfo->Current_AlternateSetting = pInfo->USBwValue0;
				return USB_SUCCESS;
			
		}
                else return  UNSUPPORT;
	}
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
	
	if (Length == 0)
		return (BYTE *)2;

	StatusInfo.w = 0;
 				/* Reset Status Information */
	
	if (Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT)) {/*Get Device Status */
		BYTE	Feature = pInfo->Current_Feature;

		if (ValBit(Feature, 5))
			SetBit(StatusInfo0, 1); /* Remote Wakeup enabled */

		if (ValBit(Feature, 7))
	        	ClrBit(StatusInfo0, 0);  /* Bus-powered */
		else if (ValBit(Feature, 6))
			SetBit(StatusInfo0, 0);  /* Self-powered */
	}
	else if (Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT))/*Interface Status*/
			return (BYTE *)&StatusInfo;

	else if (Type_Recipient == (STANDARD_REQUEST | ENDPOINT_RECIPIENT)) {/*Get EndPoint Status*/
		BYTE	Related_Endpoint;
		BYTE	wIndex0 = pInfo->USBwIndex0;

		Related_Endpoint = (wIndex0 & 0x0f);
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
	WORD    Status;

	
	if ( Type_Rec == (STANDARD_REQUEST | DEVICE_RECIPIENT) ) {/*Device Clear Feature*/
		ClrBit(pInfo->Current_Feature, 5);
		return USB_SUCCESS;
	}
	else if ( Type_Rec == (STANDARD_REQUEST | ENDPOINT_RECIPIENT) ) {/*EndPoint Clear Feature*/
		DEVICE* pDev;
		BYTE	Related_Endpoint;
		BYTE	wIndex0;
		BYTE	rEP;

		if (pInfo->USBwValue != ENDPOINT_STALL || pInfo->USBwIndex1!=0)
			return UNSUPPORT;

		pDev = &Device_Table;
		wIndex0 = pInfo->USBwIndex0;
		rEP = wIndex0 & ~0x80;
		Related_Endpoint = ENDP0 + rEP;

		if (ValBit(pInfo->USBwIndex0, 7)) Status =_GetEPTxStatus(Related_Endpoint);// get Status of endpoint & stall the request if
											   // the related_ENdpoint is Disabled
		else Status =_GetEPRxStatus(Related_Endpoint);

		if (rEP >= pDev->Total_Endpoint	|| Status==0 || pInfo->Current_Configuration==0)
			return UNSUPPORT;

	
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
		return USB_SUCCESS;
	}

	return UNSUPPORT;
} /* Standard_ClearFeature */

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	NAME:	Standard_SetEndPointFeature
	INPUT:
	OUTPUT:	none
	DESCRIPTION:
		Set or enable a specific feature of EndPoint
 *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

RESULT Standard_SetEndPointFeature(void)
{
	DEVICE_INFO	*pInfo = pInformation;
	BYTE	wIndex0;
	BYTE    Related_Endpoint;
	BYTE	rEP;
	WORD    Status;

		wIndex0 = pInfo->USBwIndex0;
		rEP = wIndex0 & ~0x80;
		Related_Endpoint = ENDP0 + rEP;

		if (ValBit(pInfo->USBwIndex0, 7)) Status =_GetEPTxStatus(Related_Endpoint);// get Status of endpoint & stall the request if
											   // the related_ENdpoint is Disabled
		else Status =_GetEPRxStatus(Related_Endpoint);



		if (Related_Endpoint >= Device_Table.Total_Endpoint || pInfo->USBwValue !=0 || Status==0 ||
			pInfo->Current_Configuration==0 /*&& Related_Endpoint!=ENDP0)*/)
			return UNSUPPORT;
		else {

			if (wIndex0 & 0x80) {		/* IN endpoint */
				_SetEPTxStatus(Related_Endpoint, EP_TX_STALL);
			}
			 	
			else {				/* OUT endpoint */
				_SetEPRxStatus(Related_Endpoint, EP_RX_STALL);
			}
		}

	return USB_SUCCESS;
} /*Standard_SetEndPointFeature */

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	NAME:	Standard_SetDeviceFeature
	INPUT:
	OUTPUT:	none
	DESCRIPTION:
		Set or enable a specific feature of Device
 *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

RESULT Standard_SetDeviceFeature(void)
{
			SetBit(pInformation->Current_Feature, 5);
			return USB_SUCCESS;

} /*Standard_SetDeviceFeature */

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

	if(pEPinfo->Usb_rLength !=0){
		vSetEPRxStatus(EP_RX_VALID);/* reenable for next data reception */
		_SetEPTxCount(ENDP0, 0);
		vSetEPTxStatus(EP_TX_VALID);/* Expect the host to abort the data OUT stage */
	}
/* Set the next State*/
	if (pEPinfo->Usb_rLength >= pEPinfo->PacketSize)
		pInformation->ControlState = OUT_DATA;
	else
		{
			if (pEPinfo->Usb_rLength >0)
				pInformation->ControlState = LAST_OUT_DATA;
			else if (pEPinfo->Usb_rLength == 0)
				{
				pInformation->ControlState = WAIT_STATUS_IN;
				USB_StatusIn();
				}
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
	
	
	if (save_wLength == 0) { /* no more data to send so STALL the TX Status*/
		ControlState = WAIT_STATUS_OUT;
		vSetEPTxStatus(EP_TX_STALL);
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

	USB_StatusOut();/* Expect the host to abort the data IN stage */

	
Expect_Status_Out:
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
	RESULT	Result = USB_SUCCESS;
	BYTE	RequestNo = pInformation->USBbRequest;
	BYTE	ControlState;
	

	if (Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT)) {/* Device Request*/
		
		/* SET_CONFIGURATION*/
	        if (RequestNo == SET_CONFIGURATION)
			Result = Standard_SetConfiguration();
	
		/*SET ADDRESS*/
		else if (RequestNo == SET_ADDRESS){
			if(pInfo->USBwValue0 > 127 || pInfo->USBwValue1!=0
				||pInfo->USBwIndex!=0 || pInfo->Current_Configuration!=0) /* Device Address should be 127 or less*/
											  /*call Beck spec USB 2.0*/
				{		
				ControlState = STALLED;
				goto exit_NoData_Setup0;
				}
			else Result = USB_SUCCESS;
			}
		/*SET FEATURE for Device*/
		else if (RequestNo == SET_FEATURE){
				if (pInfo->USBwValue0==DEVICE_REMOTE_WAKEUP && pInfo->USBwIndex==0 && pInfo->Supp_Remote_Wakeup==1)
				Result = Standard_SetDeviceFeature();
				else
				Result = UNSUPPORT;
			}
		/*Clear FEATURE for Device */
		else if (RequestNo == CLEAR_FEATURE){
			if (pInfo->USBwValue0==DEVICE_REMOTE_WAKEUP && pInfo->USBwIndex==0 && pInfo->Supp_Remote_Wakeup==1)
			Result = Standard_ClearFeature();
			else
			Result = UNSUPPORT;
		}

         }
	
	/* Interface Request*/

	else if (Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT)) {	
			
			/*SET INTERFACE*/
			if (RequestNo == SET_INTERFACE)
				Result = Standard_SetInterface();
	}

	
	/* EndPoint Request*/

	else if (Type_Recipient == (STANDARD_REQUEST | ENDPOINT_RECIPIENT)) {
			/*CLEAR FEATURE for EndPoint*/
			if (RequestNo == CLEAR_FEATURE)
			 Result = Standard_ClearFeature();
			/* SET FEATURE for EndPoint*/
			 else if (RequestNo == SET_FEATURE){
				Result = Standard_SetEndPointFeature();
			}
	}
	else
			Result = UNSUPPORT;


	if (Result != USB_SUCCESS) {
		Result = (*pProperty->Class_NoData_Setup)(RequestNo);
		if (Result == NOT_READY) {
			ControlState = PAUSE;
			goto exit_NoData_Setup0;
		}
	}

	if (Result != USB_SUCCESS) {
		ControlState = STALLED;
		goto exit_NoData_Setup0;
	}

	ControlState = WAIT_STATUS_IN;/* After no data stage SETUP */

	USB_StatusIn();

exit_NoData_Setup0:
	pInfo->ControlState = ControlState;
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
	RESULT	Result;
	BYTE Request_No = pInfo->USBbRequest;
	BYTE *pbLen;
	BYTE Related_Endpoint,Reserved;
	WORD	wOffset,wLen,Status;



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

	/*GET STATUS*/
	else if (Request_No == GET_STATUS && pInfo->USBwValue==0 && pInfo->USBwLength == 0x0002 && pInfo->USBwIndex1==0) {
			/* GET STATUS for Device*/
		if (Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT) && pInfo->USBwIndex==0) {
			CopyRoutine = Standard_GetStatus;
		}
		
			/* GET STATUS for Interface*/
		else if (Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT)){
			if ((*pProp->Class_Get_Interface_Setting)(pInfo->USBwIndex0,0)==USB_SUCCESS && pInfo->Current_Configuration!=0)
					 CopyRoutine = Standard_GetStatus;
		}
		
			/* GET STATUS for EndPoint*/
		else if (Type_Recipient == (STANDARD_REQUEST | ENDPOINT_RECIPIENT)){

			Related_Endpoint = (pInfo->USBwIndex0 & 0x0f);
			Reserved= pInfo->USBwIndex0 & 0x70;

			if (ValBit(pInfo->USBwIndex0, 7)) Status =_GetEPTxStatus(Related_Endpoint);// get Status of endpoint & stall the request if
												   // the related_ENdpoint is Disabled
			else Status =_GetEPRxStatus(Related_Endpoint);
	
			

			if(Related_Endpoint < Device_Table.Total_Endpoint && Reserved==0 && Status != 0)
					CopyRoutine = Standard_GetStatus;
		}


	}
		/*GET CONFIGURATION*/
	else if (Request_No == GET_CONFIGURATION) {
		if ( Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT) )
			CopyRoutine = Standard_GetConfiguration;
	}
		/*GET INTERFACE*/
	else if (Request_No == GET_INTERFACE) {
		if (Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT) && pInfo->Current_Configuration!=0
		   && pInfo->USBwValue==0 && pInfo->USBwIndex1==0 && pInfo->USBwLength == 0x0001
		   && (*pProperty->Class_Get_Interface_Setting)(pInfo->USBwIndex0,0)==USB_SUCCESS)			
			CopyRoutine = Standard_GetInterface;
			
	}


	if (CopyRoutine) {
		pInfo->Ctrl_Info.Usb_wOffset = wOffset;
		pInfo->Ctrl_Info.CopyData = CopyRoutine;
		/* sb in the original the cast to word was directly */
		/* now the cast is made step by step */
		pbLen = (*CopyRoutine)(0);
		wLen = (WORD)((DWORD)pbLen);
		pInfo->Ctrl_Info.Usb_wLength = wLen;
		Result = USB_SUCCESS;
	}
	else {
		Result = (*pProp->Class_Data_Setup)(pInfo->USBbRequest);
		if (Result == NOT_READY) {
			pInfo->ControlState = PAUSE;
			return;
		}
	}

	if (pInfo->Ctrl_Info.Usb_wLength == 255) {	/* Data is not ready, wait it */
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
		vSetEPRxStatus(EP_RX_VALID);/* enable for next data reception */	
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
//	vSetEPTxStatus(EP_TX_NAK);
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
	
	if (ControlState == IN_DATA || ControlState == LAST_IN_DATA)
	{
		DataStageIn();
		ControlState = pInfo->ControlState; /* may be changed outside the function */
	}
	
	else if (ControlState == WAIT_STATUS_IN) {
		if (pInfo->USBbRequest == SET_ADDRESS &&
				Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT) )
		{
			SetDeviceAddress(pInfo->USBwValue0);
		}
		(*pProperty->Process_Status_IN)();
		ControlState = STALLED;
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
	
	if(ControlState == OUT_DATA || ControlState == LAST_OUT_DATA)
		DataStageOut();

	else if (ControlState == WAIT_STATUS_OUT) {
		(*pProperty->Process_Status_OUT)();	
		ControlState = STALLED;
		}

	else if (ControlState == IN_DATA || ControlState == LAST_IN_DATA) {
		/* host aborts the transfer before finish */
				ControlState = STALLED;
	}


	/* Unexpect state, STALL the endpoint */
	else {
		
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

/******************* (C) COPYRIGHT 2006 STMicroelectronics *****END OF FILE****/

