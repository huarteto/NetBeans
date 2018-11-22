/*
* Automotive Body/DashBoard Application MCU,S3F4A0K,
* Developed by Microcontroller Design Team
* 
* Copyright 2005 by MCU System Team, Samsung Electronics CO.,LTD.
* San #24 Nongseo-Dong,Giheung-Gu,
* 449-711 Youngin-City, Gyeongi-Do, Korea.
* All rights reserved.
*
* This software is the confidential and proprietary information of Samsung
* Electronics CO.,LTD. ("Confidential Information"). 
* You shall not disclose such Confidential Inforamtion and shall use it only
* in accordance with the terms of the license agreement you entered into
* with Samsung.
* 
* The software is delivered "AS IS" without warranty or condition of any 
* kind, either express, implied or statutory. This includes without limitation
* any warranty or condition with respect to merchantability or fitness for
* any particular purpose, or against the infringements of intellectual property
* rights of others. 
*/

/**
* @file name	csp_epc.c
* @description	Function declarations for EPC module
*                
* @author	Juil Kim(82-31-209-2379, prodakim@samsung.com)   
*			Younghee Jin(82-31-209-4956, como.jin@samsung.com)
*			Minseok Jeong(82-31-209-3865, ms3525.jeong@samsung.com)		
*
* @version	Preliminary 0.0  
* @history	History type - NEW/MODify/ADD/DELete  
*  			 -------------------------------------------------------------------
*			|ver type when       who                       what             
*			|---+---+----------+-------------------------+----------------------
*			|0.0 NEW 01/07/2005  Samsung MCU System Team   Creation			
*	 		 -------------------------------------------------------------------
*
* @see	ADS Compiler Tool
*		Module/Drivers/Processor/Board -
*		EPC/ /S3F4A0K/AGP1 EVB Rev1.0
*/ 
 
/*******************************************************************************
***************************  I N C L U D E S  **********************************
*******************************************************************************/
#include "csp.h"

/*******************************************************************************
****************************  F U N C T I O N S  *******************************
*******************************************************************************/
/**
********************************************************************************
* @description	Switch on the clock, reset the registers 
*
* @return	None
*
* @param    epc	[in] Pointer to EPC structure
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_EPCInit(CSP_EPC_T *const epc)
{
	/* Enable EPC Clock */
   	CSP_EPC_SET_ECR(epc, EPC_EPC);
 
   	/* EPC Software Reset */
   	CSP_EPC_SET_CR(epc, EPC_SWRSTEPC);
}

/**
********************************************************************************
* @description	Reset and switch off the clock 
*
* @return	None
*
* @param    epc	[in] Pointer to EPC structure
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_EPCClose(CSP_EPC_T *const epc)
{
	/* EPC Software Reset */
   	CSP_EPC_SET_CR(epc, EPC_SWRSTEPC);

   	/* Disable EPC Clock */
   	CSP_EPC_SET_DCR(epc, EPC_EPC);
}

/**
********************************************************************************
* @description	Read the Power Management status 
*
* @return	32-bit value of pin data status
*
* @param    epc	[in] Pointer to EPC structure
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
U32_T CSP_EPCPowerManagementGetStatus(CSP_EPC_T *const epc)
{
	/* Return Power Management State */
   	return (CSP_EPC_GET_PMSR(epc));
}

/**
********************************************************************************
* @description	EPC Clock Settings 
*
* @return	None
*
* @param    epc			[in] Pointer to EPC structure
*			prMask_u32	[in] Clock Settings	
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_EPCSetPrescalar(CSP_EPC_T *const epc, U32_T prMask_u32)
{
	/* Write Clock Settings */
   	CSP_EPC_SET_PR(epc, prMask_u32);
}

/**
********************************************************************************
* @description	EPC Clock Settings 
*
* @return	32-bit value of clock setting
*
* @param    epc			[in] Pointer to EPC structure
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
U32_T CSP_EPCGetPrescalar(CSP_EPC_T *const epc)
{
	/* Read Clock Settings */
   	return (CSP_EPC_GET_PR(epc));
}

/**
********************************************************************************
* @description	EPC Device Settings
*
* @return	None
*
* @param    epc			[in] Pointer to EPC structure
*			dsrMask_u32	[in] Table Settings
*			xcsMask_u8	[in] Device to be Selected
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_EPCSetDeviceSelect(CSP_EPC_T *const epc, 
                           		U32_T dsrMask_u32, 
                            	U8_T xcsMask_u8)
{
	/* Write Device Settings */
   	CSP_EPC_SET_DSR(epc, xcsMask_u8, dsrMask_u32);
}

/**
********************************************************************************
* @description	EPC Device Settings
*
* @return	32-bit value of device setting
*
* @param    epc			[in] Pointer to EPC structure
*			xcsMask_u8	[in] Device to be Selected
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
U32_T CSP_EPCGetDeviceSelect(CSP_EPC_T *const epc, U8_T xcsMask_u8)
{
	/* Read Device Settings */
   	return (CSP_EPC_GET_DSR(epc, xcsMask_u8));
}

/**
********************************************************************************
* @description	Enable the write in a table
*
* @return	None
*
* @param    epc	[in] Pointer to EPC structure
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_EPCWriteEnable(CSP_EPC_T *const epc)
{
	/* Set Write */
   	CSP_EPC_SET_WER(epc, (EPC_WRITERAM));
}

/**
********************************************************************************
* @description	Disable the write in a table
*
* @return	None
*
* @param    epc	[in] Pointer to EPC structure
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_EPCWriteDisable(CSP_EPC_T *const epc)
{
	/* Clear Write */
   	CSP_EPC_SET_WDR(epc, EPC_WRITERAM);
}

/**
********************************************************************************
* @description	Get EPC Write Status
*
* @return	a 32-bit value of Write table status
*
* @param    epc	[in] Pointer to EPC structure
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
U32_T CSP_EPCWriteGetStatus(CSP_EPC_T *const epc)
{
	/* Get Status */
   	return (CSP_EPC_GET_WSR(epc));
}

/**
********************************************************************************
* @description	EPC Read Table programmation
*
* @return	None
*
* @param    epc				[in] Pointer to EPC structure
*			tableMask_u32	[in] Table Number
*			lineMask_u32	[in] Line Number
*			valMask_u32		[in] Value Number
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_EPCSetReadTable(CSP_EPC_T *const epc, 
	                         U32_T tableMask_u32, 
    	                     U32_T lineMask_u32, 
        	                 U32_T valMask_u32)
{
	/* Write RTable Settings */
   	CSP_EPC_SET_RAM_TBLR(epc, tableMask_u32, lineMask_u32, valMask_u32);
}

/**
********************************************************************************
* @description	EPC Read Table setting
*
* @return	A 32-bit value of Read table settings
*
* @param    epc				[in] Pointer to EPC structure
*			tableMask_u32	[in] Table Number
*			lineMask_u32	[in] Line Number
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
U32_T CSP_EPCGetReadTable(CSP_EPC_T *const epc, 
	                         	U32_T tableMask_u32, 
   								U32_T lineMask_u32)
{
	/* Read RTable Settings */
   	return (CSP_EPC_GET_RAM_TBLR(epc, tableMask_u32, lineMask_u32));
}

/**
********************************************************************************
* @description	EPC Write Table programmation
*
* @return	None
*
* @param    epc				[in] Pointer to EPC structure
*			tableMask_u32	[in] Table Number
*			lineMask_u32	[in] Line Number
*			valMask_u32		[in] Value Number 
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_EPCSetWriteTable(CSP_EPC_T *const epc, 
    							U32_T tableMask_u32, 
                          		U32_T lineMask_u32, 
                         		U32_T valMask_u32)
{
	/* Write WTable Settings */
  	CSP_EPC_SET_RAM_TBLW(epc, tableMask_u32, lineMask_u32, valMask_u32);
}

/**
********************************************************************************
* @description	EPC Write Table setting
*
* @return	A 32-bit value of Write table settings
*
* @param    epc				[in] Pointer to EPC structure
*			tableMask_u32	[in] Table Number
*			lineMask_u32	[in] Line Number
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
U32_T CSP_EPCGetWriteTable(CSP_EPC_T *const epc,
                         		U32_T tableMask_u32, 
                           		U32_T lineMask_u32)
{
	/* Read WTable Settings */
   	return (CSP_EPC_GET_RAM_TBLW(epc, tableMask_u32, lineMask_u32));
}

/**
********************************************************************************
* @description	EPC Init Read Table
*
* @return	None
*
* @param    epc				[in] Pointer to EPC structure
*			tableMask_u32	[in] Table Number
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_EPCInitReadTable(CSP_EPC_T *const epc, U32_T tableMask_u32)
{
	CSP_EPCSetReadTable(epc, tableMask_u32, 0,
    					EPC_CTRL4_LINE(EPC_CTRL_16|EPC_CTRL_8H|EPC_CTRL_8L)|
                       	EPC_CTRL3_LINE(EPC_CTRL_16|EPC_CTRL_8H|EPC_CTRL_8L)|
                       	EPC_CTRL2_LINE(EPC_CTRL_16|EPC_CTRL_8H|EPC_CTRL_8L)|
                       	EPC_CTRL1_LINE(EPC_CTRL_16|EPC_CTRL_8H|EPC_CTRL_8L)|
                       	EPC_CTRL0_LINE(EPC_CTRL_16|EPC_CTRL_8H|EPC_CTRL_8L)|
                       	EPC_XCS|
                       	EPC_VALIDDATA|
                       	EPC_EOT|
                       	EPC_CYCLECOUNTER((U8_T)0x1));
      
	CSP_EPCSetReadTable(epc, tableMask_u32, 1, 
    					EPC_CTRL4_LINE(EPC_CTRL_16|EPC_CTRL_8H|EPC_CTRL_8L)|
                       	EPC_CTRL3_LINE(EPC_CTRL_16|EPC_CTRL_8H|EPC_CTRL_8L)|
                       	EPC_CTRL2_LINE(EPC_CTRL_16|EPC_CTRL_8H|EPC_CTRL_8L)|
                       	EPC_CTRL1_LINE(EPC_CTRL_16|EPC_CTRL_8H|EPC_CTRL_8L)|
                       	EPC_CTRL0_LINE(EPC_CTRL_16|EPC_CTRL_8H|EPC_CTRL_8L)|
                       	EPC_XCS|
                       	EPC_VALIDDATA|
                       	EPC_CYCLECOUNTER((U8_T)0x00));

}

/**
********************************************************************************
* @description	EPC Init Write Table
*
* @return	None
*
* @param    epc				[in] Pointer to EPC structure
*			tableMask_u32	[in] Table Number
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_EPCInitWriteTable(CSP_EPC_T *const epc, U32_T tableMask_u32)
{
	CSP_EPCSetWriteTable(epc, tableMask_u32, 0, 
    					EPC_CTRL4_LINE(EPC_CTRL_16|EPC_CTRL_8H|EPC_CTRL_8L)|
                        EPC_CTRL3_LINE(EPC_CTRL_16|EPC_CTRL_8H|EPC_CTRL_8L)|
                        EPC_CTRL2_LINE(EPC_CTRL_16|EPC_CTRL_8H|EPC_CTRL_8L)|
                        EPC_CTRL1_LINE(EPC_CTRL_16|EPC_CTRL_8H|EPC_CTRL_8L)|
                        EPC_CTRL0_LINE(EPC_CTRL_16|EPC_CTRL_8H|EPC_CTRL_8L)|
                        EPC_XCS|
                        EPC_VALIDDATA|
                        EPC_EOT|
                        EPC_CYCLECOUNTER((U8_T)0x1));
      
	CSP_EPCSetWriteTable(epc, tableMask_u32, 1, 
    					EPC_CTRL4_LINE(EPC_CTRL_16|EPC_CTRL_8H|EPC_CTRL_8L)|
                        EPC_CTRL3_LINE(EPC_CTRL_16|EPC_CTRL_8H|EPC_CTRL_8L)|
                        EPC_CTRL2_LINE(EPC_CTRL_16|EPC_CTRL_8H|EPC_CTRL_8L)|
                        EPC_CTRL1_LINE(EPC_CTRL_16|EPC_CTRL_8H|EPC_CTRL_8L)|
                        EPC_CTRL0_LINE(EPC_CTRL_16|EPC_CTRL_8H|EPC_CTRL_8L)|
                        EPC_XCS|
                        EPC_VALIDDATA|
                        EPC_CYCLECOUNTER((U8_T)0x00));
	
}

/**
********************************************************************************
* @description	EPC Clear Read Table
*
* @return	None
*
* @param    epc				[in] Pointer to EPC structure
*			tableMask_u32	[in] Table Number
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_EPCClearReadTable(CSP_EPC_T *const epc, U32_T tableMask_u32)
{
	/* Local variable */ 
   	U8_T line_u8;
   
   	/* Clear Read table */
   	for (line_u8 = 0;line_u8 < 8;line_u8++)
   	{
    	CSP_EPCSetReadTable(epc, tableMask_u32, line_u8, 0x00000000);
   	}
}

/**
********************************************************************************
* @description	EPC Clear Write Table
*
* @return	None
*
* @param    epc				[in] Pointer to EPC structure
*			tableMask_u32	[in] Table Number
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_EPCClearWriteTable(CSP_EPC_T *const epc, U32_T tableMask_u32)
{
	/* Local variable */
   	U8_T line_u8;
   
  	/* Clear Write table */
   	for (line_u8 = 0;line_u8 < 8;line_u8++)
   	{
    	CSP_EPCSetWriteTable(epc, tableMask_u32, line_u8, 0x00000000);
   	}
}

/******************************************************************************
* EPC interrupt block section
******************************************************************************/


/**
********************************************************************************
* @description	Clear EPC status register
*
* @return	None
*
* @param    epc			[in] Pointer to EPC structure
*			pioEvent_u8	[in] Configure which interrupt block to point
*			pioMask_u32	[in] Configure which bits are cleared
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_EPCClearStatus(CSP_EPC_T *const epc,
							U8_T pioEvent_u8,
							U32_T pioMask_u32)
{
	/* Set Bit to be Cleared */
   	CSP_EPC_SET_CSR(epc, pioEvent_u8, pioMask_u32);
}

/**
********************************************************************************
* @description	Read the input pin interrupt status
*
* @return	A 32-bit value of the status register
*
* @param    epc			[in] Pointer to EPC structure
*			pioEvent_u8	[in] Configure which interrupt block to point
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
U32_T CSP_EPCInterruptGetStatus(CSP_EPC_T *const epc, U8_T pioEvent_u8)
{
	/* Return the status register */
   	return (CSP_EPC_GET_SR(epc, pioEvent_u8));
}

/**
********************************************************************************
* @description	Read the input pin interrupt status
*
* @return	A 32-bit value of the status register
*
* @param    epc			[in] Pointer to EPC structure
*			intMode_u32 [in] Configure the priority level and source type
*			intMask_u32	[in] Configure which interrupt block to point
*			pioEvent_u8 [in] Configure which interrupt bits are activated
*			callback_u32[in] Function called through the assembler interrupt 
*                            handler
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_EPCConfigInterrupt(CSP_EPC_T *const epc, 
                           		U32_T intMode_u32, 
                            	U32_T intMask_u32, 
                            	U8_T pioEvent_u8, 
                            	U32_T callback_u32)
{
	/* Local Variables */
   	U8_T intNum_u8 = 0;

   	/* Get Peripheral Interrupt Number */
   	intNum_u8 = CSP_GET_SYS_INT_NUMBER(epc);

   	/* Disable all interrupt */
   	CSP_EPC_SET_IDR(epc, 0, 0xFFFFFFFF);
   	CSP_EPC_SET_IDR(epc, 1, 0x0007FFFF);
      
   	/* Interrupt Enable */
   	CSP_EPC_SET_IER(epc, pioEvent_u8, intMask_u32);
   
   	/* Configure EPC controller interrupt mode in GIC module */
   	CSP_GICConfigInterrupt(intNum_u8, intMode_u32, callback_u32);
}

/**
********************************************************************************
* @description	Read the interrupt status
*
* @return	A 32-bit value of the status register
*
* @param    epc			[in] Pointer to EPC structure
*			pioEvent_u8 [in] Configure which interrupt block to point.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
U32_T CSP_EPCInterruptMaskGetStatus(CSP_EPC_T *const epc, U8_T pioEvent_u8)
{
	/* Return Power Interrupt Status */
   	return (CSP_EPC_GET_IMR(epc, pioEvent_u8));
}

