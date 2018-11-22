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
* @file name	csp_can.c
* @description	Function declarations for CAN module
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
*		CAN/ /S3F4A0K/AGP1 EVB Rev1.0
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
* @description	 Switch on the clock, reset registers and state machines,
*           	 initialize Message RAM and configure the CAN mode 
*
* @return	None
*
* @param    can			[in] Pointer to CAN structure.
*			mode_u32	[in] Configure the CAN mode.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_CANInit(CSP_CAN_T *const can, U32_T mode_u32)
{
	/* Local variables */
   	U8_T channel_u8;

   	/* Enable CAN Clock */  
   	CSP_CAN_SET_ECR(can, CAN_CAN);
  
   	/* CAN Software Reset */  
   	CSP_CAN_SET_CR(can, CAN_SWRST);
  
   	/* Initialize Message RAM */
   	for (channel_u8 = 1; channel_u8 <= (U8_T)NB_CAN_MO; channel_u8++)
   	{
    	while((CSP_CAN_GET_SR(can) & CAN_BUSY0) == CAN_BUSY0)
     		;/*NULL*/
		CSP_CAN_IF_SET_DAR( can, 0 , 0x00000000);
   		CSP_CAN_IF_SET_DBR( can, 0 , 0x00000000);
    	CSP_CAN_IF_SET_MSKR(can, 0 , 0xDFFFFFFF);
    	CSP_CAN_IF_SET_IR(  can, 0 , 0x00000000);
    	CSP_CAN_IF_SET_MCR( can, 0 , 0x00000000);
    	CSP_CAN_IF_SET_TMR( can, 0 , (CAN_AMCR|CAN_AIR|CAN_AMSKR|CAN_ADAR|
    									CAN_ADBR | CAN_WR | channel_u8));
 	}
   
	/* Enable configuration change */
   	CSP_CAN_SET_CR(can, CAN_CCEN);

   	/* Configure CAN Mode */
   	CSP_CAN_SET_MR(can, mode_u32);

	/* Disable configuration change */
   	CSP_CAN_SET_CR(can, CAN_CCDIS);
}

/**
********************************************************************************
* @description	 Reset and switch off the clock
*
* @return	None
*
* @param    can			[in] Pointer to CAN structure.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_CANClose(CSP_CAN_T *const can)
{
  	/* CAN Software Reset */  
   	CSP_CAN_SET_CR(can, CAN_SWRST);

   	/* Disable CAN Clock */  
   	CSP_CAN_SET_DCR(can, CAN_CAN);
}

/**
********************************************************************************
* @description	 Configure CAN Interrupts
*
* @return	None
*
* @param    can			[in] Pointer to CAN structure.
*			intMode_u32 [in] Configure the priority level and source type.
*			intMask_u32 [in] Configure which interrupt bits are activated.
*			channelMask_u32 [in] Configure which interrupt bits are activated.
*			callback_u32	[in] Function called through the assembler 
*  	  						     interrupt handler.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_CANConfigInterrupt(CSP_CAN_T *const can, 
                            U32_T intMode_u32, 
                            U32_T intMask_u32, 
                            U32_T channelMask_u32, 
                            U32_T callback_u32)
{
	/* Local Variables */
   	U8_T intNum_u8 = 0;
   	U8_T channel_u8;

   	/* Get Peripheral Interrupt Number */
   	intNum_u8 = CSP_GET_INT_NUMBER(can);

   	/* Disable all interrupts */
   	CSP_CAN_SET_IDR(can, 0xFFFFFFFF);
   	CSP_CAN_SET_SIDR(can, 0xFFFFFFFF);

   	/* Clear interrupts at CAN controller level */
   	CSP_CAN_SET_CSR(can, CAN_CSR_MASK);

   	/* Clear interrupts at Message Object level */
   	for (channel_u8 = 1; channel_u8 <= (U8_T)NB_CAN_MO; channel_u8++)
   	{
    	while((CSP_CAN_GET_SR(can) & CAN_BUSY0) == CAN_BUSY0)
			; /*NULL*/
    	CSP_CAN_IF_SET_TMR(can , 0 ,(CAN_ITPND | channel_u8));
	}

   	/* Enable interrupt at CAN controller level */
   	CSP_CAN_SET_IER(can, intMask_u32);

   	/* Enable interrupt at Message Object level */
   	CSP_CAN_SET_SIER(can, channelMask_u32);

   	/* Configure CAN controller interrupt mode in GIC module */
   	CSP_GICConfigInterrupt(intNum_u8, intMode_u32, callback_u32);
   	
}
                     

/**
********************************************************************************
* @description	 Enable CAN
*
* @return	None
*
* @param    can			[in] Pointer to CAN structure.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_CANEnable(CSP_CAN_T *const can)
{
   /* Enable CAN */
   CSP_CAN_SET_CR(can, CAN_CANEN);
}  

/**
********************************************************************************
* @description	 Disable CAN
*
* @return	None
*
* @param    can			[in] Pointer to CAN structure.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_CANDisable(CSP_CAN_T *const can)
{
   /* Disable CAN */
   CSP_CAN_SET_CR(can, CAN_CANDIS);
}  

/**
********************************************************************************
* @description	Configuration a CAN Message Object
*				To pick up message from the network
*
* @return	None
*
* @param    can 	     [in] Pointer to CAN structure.
*			interface_u8 [in] Interface registers used to transfer to MessageRAM
*			channel_u8	 [in] Receiver channel [0-31].
*			id_u32		 [in] Configure the identifier, the frame format and
* 							  Direction
*                             XTD  : extend format
*                             MDIR : wait a remote frame
*			mask_u32	 [in] Configure the mask,
*							  Format and direction mask.
*			control_u16	 [in] Configure the number of byte to receive.
*                             Configure the control bits.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_CANReceive(CSP_CAN_T *const can, 
                      	U8_T  interface_u8, 
                       	U8_T  channel_u8, 
                       	U32_T id_u32,
                       	U32_T mask_u32, 
                       	U16_T control_u16)
{
	/* Set Id Register */
   	CSP_CAN_IF_SET_IR(can, interface_u8, CAN_MSGVAL | id_u32);

   	/* Set Mask Register */
   	CSP_CAN_IF_SET_MSKR(can, interface_u8, mask_u32); 

   	/* Set Control Register */
   	CSP_CAN_IF_SET_MCR(can, interface_u8, control_u16);

   	/* Set message object 'channel' with interface registers */
   	CSP_CAN_IF_SET_TMR(can,
   						interface_u8, 
   						(CAN_AMCR | CAN_AIR | CAN_AMSKR |CAN_WR | channel_u8));

	/* Wait before the transfer between interface registers and Message */
	/* Object RAM memory has completed */
   	while (((CSP_CAN_GET_SR(can) & CAN_BUSY0) == CAN_BUSY0) || 
    		((CSP_CAN_GET_SR(can) & CAN_BUSY1) == CAN_BUSY1))
  				; /*NULL*/
  
}

/**
********************************************************************************
* @description	Configure a CAN MessageObject to transmit message on the network
*
* @return	None
*
* @param    can 	     [in] Pointer to CAN structure.
*			interface_u8 [in] Interface registers used to transfer to MessageRAM
*			channel_u8	 [in] Transmitter channel [0-31].
*			dataa_u32	 [in] Pointer to an array where data packets are stored.
*			datab_u32	 [in] Pointer to an array where data packets are stored.	
*			id_u32		 [in] Configure the identifier, the frame format and
*    						  The remote request
*			mask_u32	 [in] Configure the mask,
*							  Format and direction mask.
*			control_u16	 [in] Configure the number of byte to transmit
*						      Configure the control bits.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_CANTransmit(CSP_CAN_T *const can, 
                        U8_T  interface_u8, 
                        U8_T  channel_u8, 
                        U32_T dataa_u32,
                        U32_T datab_u32,
                        U32_T id_u32,
                        U16_T control_u16)
{
	/* Set DATAA Register */
   	CSP_CAN_IF_SET_DAR(can, interface_u8, dataa_u32);

	/* Set DATAB Register */
   	CSP_CAN_IF_SET_DBR(can, interface_u8, datab_u32);

	/* Set Id Register */
   	CSP_CAN_IF_SET_IR(can, interface_u8, CAN_MSGVAL | id_u32);

	/* Set Control Register */
   	CSP_CAN_IF_SET_MCR(can, interface_u8, control_u16);

   	/* Set message object 'channel' with interface registers */
   	CSP_CAN_IF_SET_TMR(can,
   						 interface_u8, 
   						(CAN_TRND| CAN_AMCR| CAN_AIR| 
						 CAN_ADBR| CAN_ADAR| CAN_WR | channel_u8));

	/* Wait before the transfer between interface registers and Message */
	/* Object RAM memory has completed */
   	while (((CSP_CAN_GET_SR(can) & CAN_BUSY0) == CAN_BUSY0) || 
     		((CSP_CAN_GET_SR(can) & CAN_BUSY1) == CAN_BUSY1))
   				; /*NULL*/
   	
}
/**
********************************************************************************
* @description	Configure a CAN MessageObject to transmit message on the network
*
* @return	None
*
* @param    can 	     [in] Pointer to CAN structure.
*			coreClk_u32  [in] ICore clock frequency expressed in Hz.
*			baudRate_u32 [in] CAN baud rate desired expressed in bits/s.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
U32_T CSP_CANComputeBT(CSP_CAN_T *const can, 
                      		U32_T   coreClk_u32, 
                      		U32_T   baudRate_u32)
{
	/* Locals variables used for bit timing computation */
   	U32_T	mode_u32;
   	U32_T   baudRatePrescalar_u32;
   	U32_T   bitTimeLength_u32;	   		     
   	U32_T   bitTimeLengthTenth_u32;
   	U32_T   propSegment_u32;   		     
   	U32_T   phaseSegments_u32;   		     
   	U32_T   phaseSegment1_u32;      		     
   	U32_T   phaseSegment2_u32;   		     
   	U32_T   sjwSegment_u32;
   	U8_T    error_u8 = 0;

	if ((((100 * coreClk_u32)/baudRate_u32) - 
			(100 * (coreClk_u32/baudRate_u32))) != 0)
	{
    	error_u8 = 1;
	}
	
	/* The bit time may consist of 4 to 25time quanta tq,
	 * where tq = (BD[9:0] / CoreClk) 
	 */
	baudRatePrescalar_u32 = 0;
   	bitTimeLength_u32 = coreClk_u32 / baudRate_u32;
   	bitTimeLengthTenth_u32 = (10 * coreClk_u32) / baudRate_u32;

	/* BitTimeLength expressed in tq may be at max of 25 
	 * and may be an integer multiple 
	 */
  	while ((bitTimeLength_u32 > 25) |
  			(bitTimeLengthTenth_u32 != (10 * bitTimeLength_u32)))     
   	{
    	baudRatePrescalar_u32 = baudRatePrescalar_u32 + 1;
      	/* It's rouded. */
      	bitTimeLength_u32 = coreClk_u32 / baudRate_u32 / 
        	                 (baudRatePrescalar_u32 + 1);  
      	/* Here, the tenth accuracy is keeped */           
      	bitTimeLengthTenth_u32 = (10 * coreClk_u32) / baudRate_u32 / 
        	                       (baudRatePrescalar_u32 + 1); 
   	}
   	if (bitTimeLength_u32 < 4) 
   	{
		/* A bit time is at min 4 tq */
   		error_u8 = 1; 
   	}

  	propSegment_u32 = 1;
   	while ((coreClk_u32 / (baudRatePrescalar_u32 + 1) / 
    	     propSegment_u32) > 3333333)
	/* coreClk_u32 / (baudRatePrescalar_u32 + 1) => 1/tq */
 	{   
    	propSegment_u32 = propSegment_u32 + 1;
   	}
   
   	/* (Prop_seg + Sync_seg) may be < (Bit time length - 1) */
   	if ((propSegment_u32 + 1) >= (bitTimeLength_u32-1))
	{
		error_u8 = 1;  
   	}		
   
   	/* The two phase segments are (Bit time - Prop_seg - Sync_seg) long 
   	 * If (Bit time - Prop_seg - Sync_seg) is even : 
   	 * Phase_seg2 = Phase_seg1 else Phase_seg2 = (Phase_seg1 + 1) 
   	 * The IPT Information processing time is 0 tq. 
   	 * Phase_seg2 must be > IPT : no problem 
   	 */
   	phaseSegments_u32 = bitTimeLength_u32 - propSegment_u32 - 1;  
   	phaseSegment1_u32 = phaseSegments_u32 / 2;
   	phaseSegment2_u32 = phaseSegments_u32 - phaseSegment1_u32;
   	phaseSegment1_u32 = phaseSegment1_u32 + propSegment_u32;
   
   	/* There are only 3 bits to code phase seg2 length : 
   	 * then it can not exceed 8
   	 */
   	while (phaseSegment2_u32 > 8) 
   	{
    	phaseSegment2_u32 = phaseSegment2_u32 - 1;
      	phaseSegment1_u32 = phaseSegment1_u32 + 1;
	}   

   	/* The length of the Synchronization Jump Width is the smaller  
   	 * of 4 and Phase_seg1 
   	 */
   	if ((phaseSegment1_u32 - propSegment_u32) < 4)
   	{
    	sjwSegment_u32 = (phaseSegment1_u32 - propSegment_u32);
   	}
   	else
   	{
    	sjwSegment_u32 = 4;
   	}

   	/* Store MR value */
   	mode_u32 = baudRatePrescalar_u32 | (sjwSegment_u32 - 1)<<12 | CAN_AR | 
				(phaseSegment1_u32 - 1)<<16 | (phaseSegment2_u32 - 1)<<20;
      
   	if(error_u8 != 0)
   	{
    	mode_u32 = 0;
   	}

   	/* Return */
   	return(mode_u32);
}


