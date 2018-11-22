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
* @file name	csp.c
* @description	Structure for S3F4A0K Chip.
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
*		S3F4A0K AGP1
*/ 
/*******************************************************************************
***************************  I N C L U D E S  **********************************
*******************************************************************************/
#include "csp.h"

/*******************************************************************************
****************  P E R I P H E R A L   D E C L A R A T I O N  *****************
*******************************************************************************/
/** Internal RAM Controller                                                    */
CSP_IRC_T      *const IRC     = ((CSP_IRC_T*)      	IRC_BASE_ADDRESS);   
/** Generic Interrupt Controller                                               */
CSP_GIC_T      *const GIC     = ((CSP_GIC_T*)      	GIC_BASE_ADDRESS);   
/**  Lite Direct Memory Access                                                 */
CSP_LDMA_T     *const LDMA0  = ((CSP_LDMA_T*)      	LDMA_BASE_ADDRESS);
/** Data Flash Controller                                                      */
CSP_DFC_T      *const DFC0    = ((CSP_DFC_T*)      	DFC0_BASE_ADDRESS);
/** Interleave Flash Controller                                                */
CSP_IFC_T      *const IFC0    = ((CSP_IFC_T*)      	IFC0_BASE_ADDRESS);
/** Pulse Width Modulation                                                     */
CSP_PWM_T      *const PWM0    = ((CSP_PWM_T*)      	PWM0_BASE_ADDRESS);
/** Analog to Digital Converter                                                */
CSP_ADC10_T    *const ADC0    = ((CSP_ADC10_T*)		ADC0_BASE_ADDRESS);
/** Serial Peripheral Interface                                                */
CSP_SPI_T      *const SPI0    = ((CSP_SPI_T*)      	SPI0_BASE_ADDRESS);
/** Watchdog                                                                   */
CSP_WD_T       *const WD0     = ((CSP_WD_T*)       	WD0_BASE_ADDRESS);
/** Controller Area Network                                                    */
CSP_CAN_T      *const CAN0    = ((CSP_CAN_T*)      	CAN0_BASE_ADDRESS);
/** General Purpose Timer 3 Channels                                           */
CSP_GPT3C_T    *const GPT0    = ((CSP_GPT3C_T*)    	GPT0_BASE_ADDRESS);
/** General Purpose Timer 1 Channel                                            */
CSP_GPT_T      *const GPT0CH0 = ((CSP_GPT_T*)      GPT0CH0_BASE_ADDRESS);
/** General Purpose Timer 1 Channel                                            */
CSP_GPT_T      *const GPT0CH1 = ((CSP_GPT_T*)      GPT0CH1_BASE_ADDRESS);
/** General Purpose Timer 1 Channel                                            */
CSP_GPT_T      *const GPT0CH2 = ((CSP_GPT_T*)      GPT0CH2_BASE_ADDRESS);
/** Simple Timer 16 bits                                                       */
CSP_ST16_T     *const ST0     = ((CSP_ST16_T*)     ST0_BASE_ADDRESS);
/** Simple Timer 16 bits                                                       */
CSP_ST16_T     *const ST1     = ((CSP_ST16_T*)     ST1_BASE_ADDRESS);
/** Universal Sync/Async receiver/Transmitter                                  */
CSP_USART_T    *const UART0   = ((CSP_USART_T*)    UART0_BASE_ADDRESS);
/** IO Configuration														   */
CSP_IO_CFG_T   *const IO_CFG0 = ((CSP_IO_CFG_T*)	IOCFG0_BASE_ADDRESS);
/** Stamp Timer                                                                */
CSP_STT_T      *const STT0    = ((CSP_STT_T*)      	STT0_BASE_ADDRESS);
/** Universal Sync/Async receiver/Transmitter                                  */
CSP_USART_T    *const UART1   = ((CSP_USART_T*)    	UART1_BASE_ADDRESS);
/** Universal Sync/Async receiver/Transmitter                                  */
CSP_USART_T    *const USART0  = ((CSP_USART_T*)		USART0_BASE_ADDRESS);
/** Controller Area Network                                                    */
CSP_CAN_T      *const CAN1    = ((CSP_CAN_T*)      	CAN1_BASE_ADDRESS);
/** Controller Area Network                                                    */
CSP_CAN_T      *const CAN2    = ((CSP_CAN_T*)      	CAN2_BASE_ADDRESS);
/** Controller Area Network                                                    */
CSP_CAN_T      *const CAN3    = ((CSP_CAN_T*)      	CAN3_BASE_ADDRESS);
/** Capture                                                                    */
CSP_CAPT_T     *const CAPT0   = ((CSP_CAPT_T*)     	CAPT0_BASE_ADDRESS);
/** Capture                                                                    */
CSP_CAPT_T     *const CAPT1   = ((CSP_CAPT_T*)     	CAPT1_BASE_ADDRESS);
/** Inter-Integrated Circuit                                                   */
CSP_I2C_T      *const I2C0    = ((CSP_I2C_T*)      	I2C0_BASE_ADDRESS);
/** Inter-Integrated Circuit                                                   */
CSP_I2C_T      *const I2C1    = ((CSP_I2C_T*)      	I2C1_BASE_ADDRESS);
/** LCD Controller                                                             */
CSP_LCDC_T     *const LCDC0   = ((CSP_LCDC_T*)		LCDC0_BASE_ADDRESS);
/** Simple Timer 16 bits                                                       */
CSP_ST16_T     *const ST2     = ((CSP_ST16_T*)		ST2_BASE_ADDRESS);
/** Serial Peripheral Interface                                                */
CSP_SPI_T      *const SPI1    = ((CSP_SPI_T*)      	SPI1_BASE_ADDRESS);
/** General Purpose I/O                                                        */
CSP_PIO_T      *const PIO0    = ((CSP_PIO_T*)      	PIO0_BASE_ADDRESS);
/** General Purpose I/O                                                        */
CSP_PIO_T      *const PIO1    = ((CSP_PIO_T*)    	PIO1_BASE_ADDRESS);
/** General Purpose I/O                                                        */
CSP_PIO_T      *const PIO2    = ((CSP_PIO_T*)      	PIO2_BASE_ADDRESS);
/** General Purpose I/O                                                        */
CSP_PIO_T      *const PIO3    = ((CSP_PIO_T*)      	PIO3_BASE_ADDRESS);
/** Stepper Motor Controller                                                   */
CSP_SMC_T      *const SMC0    = ((CSP_SMC_T*)      	SMC0_BASE_ADDRESS);
/** Stepper Motor Controller                                                   */
CSP_SMC_T      *const SMC1    = ((CSP_SMC_T*)      	SMC1_BASE_ADDRESS);
/** Stepper Motor Controller                                                   */
CSP_SMC_T      *const SMC2    = ((CSP_SMC_T*)      	SMC2_BASE_ADDRESS);
/** Stepper Motor Controller                                                   */
CSP_SMC_T      *const SMC3    = ((CSP_SMC_T*)      	SMC3_BASE_ADDRESS);
/** Stepper Motor Controller                                                   */
CSP_SMC_T      *const SMC4    = ((CSP_SMC_T*)      	SMC4_BASE_ADDRESS);
/** Stepper Motor Controller                                                   */
CSP_SMC_T      *const SMC5    = ((CSP_SMC_T*)      	SMC5_BASE_ADDRESS);
/** External Protocol Controller                                               */
CSP_EPC_T      *const EPC0    = ((CSP_EPC_T*)      	EPC0_BASE_ADDRESS);
/** Special Function Module                                                    */
CSP_SFM_T      *const SFM0    = ((CSP_SFM_T*)      	SFM0_BASE_ADDRESS);
/** Clock Manager                                                              */
CSP_CM_T       *const CM0     = ((CSP_CM_T*)       	CM0_BASE_ADDRESS);


/*******************************************************************************
***********************  GHOST PERIPHERAL DECLARATION  *************************
*************  Ghost field do not disturb the behaviour of module  *************
*******************************************************************************/
#define  BIT_13   (0x01ul << 13)
/** Generic Interrupt Controller 											   */                                               
CSP_GIC_T   *const GIC_GHOST  	= ((CSP_GIC_T*)	(GIC_BASE_ADDRESS ^ BIT_13));
/** Pulse Width Modulation Ghost                                               */                                              
CSP_PWM_T   *const PWM0_GHOST	= ((CSP_PWM_T*)	(PWM0_BASE_ADDRESS ^ BIT_13));
/** Analog to Digital Converter	                                               */                                                 
CSP_ADC10_T *const ADC0_GHOST	= ((CSP_ADC10_T*) (ADC0_BASE_ADDRESS ^ BIT_13));
/** Serial Peripheral Interface                                                */
CSP_SPI_T   *const SPI0_GHOST	= ((CSP_SPI_T*)	(SPI0_BASE_ADDRESS ^ BIT_13));
/** General Purpose Timer 3 Channels                                           */
CSP_GPT3C_T *const GPT0_GHOST	= ((CSP_GPT3C_T*) (GPT0_BASE_ADDRESS ^ BIT_13));
/** General Purpose Timer 1 Channel                                            */
CSP_GPT_T   *const GPT0CH0_GHOST	= ((CSP_GPT_T*)
											(GPT0CH0_BASE_ADDRESS ^ BIT_13));
/** General Purpose Timer 1 Channel                                            */
CSP_GPT_T   *const GPT0CH1_GHOST	= ((CSP_GPT_T*)   
                                      		(GPT0CH1_BASE_ADDRESS ^ BIT_13));
/** General Purpose Timer 1 Channel                                            */
CSP_GPT_T   *const GPT0CH2_GHOST	= ((CSP_GPT_T*)   
                                      		(GPT0CH2_BASE_ADDRESS ^ BIT_13));
/** Universal Sync/Async receiver/Transmitter                                  */
CSP_USART_T *const UART0_GHOST	= ((CSP_USART_T*) 
										(UART0_BASE_ADDRESS ^ BIT_13));
/** Stamp Timer                                                                */
CSP_STT_T   *const STT0_GHOST	= ((CSP_STT_T*)   
										(STT0_BASE_ADDRESS ^ BIT_13));
/** Universal Sync/Async receiver/Transmitter                                  */
CSP_USART_T *const UART1_GHOST	= ((CSP_USART_T*) 
                                  		(UART1_BASE_ADDRESS ^ BIT_13));
/** Universal Sync/Async receiver/Transmitter                                  */
CSP_USART_T *const USART0_GHOST	= ((CSP_USART_T*) 
                                 		(USART0_BASE_ADDRESS ^ BIT_13));
/** Capture                                                                    */
CSP_CAPT_T  *const CAPT0_GHOST	= ((CSP_CAPT_T*) (CAPT0_BASE_ADDRESS ^ BIT_13));
/** Capture                                                                    */
CSP_CAPT_T  *const CAPT1_GHOST	= ((CSP_CAPT_T*) (CAPT1_BASE_ADDRESS ^ BIT_13));
/** Serial Peripheral Interface                                                */ 
CSP_SPI_T   *const SPI1_GHOST	= ((CSP_SPI_T*) (SPI1_BASE_ADDRESS ^ BIT_13));


/*******************************************************************************
************************  C H I P   D E S C R I P T I O N  *********************
*******************************************************************************/
const PERIPHERAL_TYPE_T CHIP[PERIPH_NUMBER+1] = 
{ 
/* {Peripheral Type,     Interrupt Number[ch],Reserved,Reserved}                
 * {                 {    ch0,    ch1,    ch2}, LdmaRx,  LdmaTx},   Periph 
 */ 
   {DFC_E,           {      0, UNUSED, UNUSED}, UNUSED, UNUSED}, /**< DFC    */
   {IFC_E,           {      3, UNUSED, UNUSED}, UNUSED, UNUSED}, /**< IFC    */
   {PWM_E,           {      4, UNUSED, UNUSED}, UNUSED, UNUSED}, /**< PWM0   */      
   {ADC16C_10_E,     {      5, UNUSED, UNUSED},     12, UNUSED}, /**< ADC0   */     
   {SPI_E,           {      6, UNUSED, UNUSED},      1,      0}, /**< SPI0   */
   {WATCHDOG_E,      {      7, UNUSED, UNUSED}, UNUSED, UNUSED}, /**< WD0    */   
   {CAN32C_E,        {      8, UNUSED, UNUSED}, UNUSED, UNUSED}, /**< CAN0   */
   {GPT3C_E,         {      9,     26,     27}, UNUSED, UNUSED}, /**< GPT0   */
   {ST16_E,          {     10, UNUSED, UNUSED}, UNUSED, UNUSED}, /**< ST0    */
   {ST16_E,          {     11, UNUSED, UNUSED}, UNUSED, UNUSED}, /**< ST1    */
   {USART_E,         {     12, UNUSED, UNUSED},      5,      4}, /**< UART0  */  
   {IOCONF_E,        {      2, UNUSED, UNUSED}, UNUSED, UNUSED}, /**< IOCONF0*/
   {STT_E,           {     25, UNUSED, UNUSED}, UNUSED, UNUSED}, /**< STT0   */
   {USART_E,         {     14, UNUSED, UNUSED},      7,      6}, /**< UART1  */
   {USART_E,         {     20, UNUSED, UNUSED},      9,      8}, /**< USART0 */
   {CAN32C_E,        {     15, UNUSED, UNUSED}, UNUSED, UNUSED}, /**< CAN1   */
   {CAN32C_E,        {     16, UNUSED, UNUSED}, UNUSED, UNUSED}, /**< CAN2   */
   {CAN32C_E,        {     17, UNUSED, UNUSED}, UNUSED, UNUSED}, /**< CAN3   */
   {CAPTURE_E,       {     18, UNUSED, UNUSED},     10, UNUSED}, /**< CAPT0  */
   {CAPTURE_E,       {     19, UNUSED, UNUSED},     11, UNUSED}, /**< CAPT1  */
   {I2C_E,           {     13, UNUSED, UNUSED}, UNUSED, UNUSED}, /**< I2C0   */
   {I2C_E,           {     21, UNUSED, UNUSED}, UNUSED, UNUSED}, /**< I2C1   */
   {LCDC_E,          { UNUSED, UNUSED, UNUSED}, UNUSED, UNUSED}, /**< LCDC   */
   {ST16_E,          {     23, UNUSED, UNUSED}, UNUSED, UNUSED}, /**< ST2    */   
   {SPI_E,           {     24, UNUSED, UNUSED},      3,      2}, /**< SPI1   */
   {PIO_E,           {     29, UNUSED, UNUSED}, UNUSED, UNUSED}, /**< PIO0   */
   {PIO_E,           {     30, UNUSED, UNUSED}, UNUSED, UNUSED}, /**< PIO1   */
   {PIO_E,           {     31, UNUSED, UNUSED}, UNUSED, UNUSED}, /**< PIO2   */
   {PIO_E,           {     32, UNUSED, UNUSED}, UNUSED, UNUSED}, /**< PIO3   */
   {SMC_E,           {     33, UNUSED, UNUSED}, UNUSED, UNUSED}, /**< SMC0   */
   {SMC_E,           {     34, UNUSED, UNUSED}, UNUSED, UNUSED}, /**< SMC1   */
   {SMC_E,           {     35, UNUSED, UNUSED}, UNUSED, UNUSED}, /**< SMC2   */
   {SMC_E,           {     36, UNUSED, UNUSED}, UNUSED, UNUSED}, /**< SMC3   */  
   {SMC_E,           {     37, UNUSED, UNUSED}, UNUSED, UNUSED}, /**< SMC4   */
   {SMC_E,           {     38, UNUSED, UNUSED}, UNUSED, UNUSED}, /**< SMC5   */     
   {UNUSED_PERIPHERAL_TYPE_E,{ UNUSED, UNUSED, UNUSED}, UNUSED, UNUSED},
};

/*******************************************************************************
******************  SYSTEM PERIPHERAL INTERRUPT DESCRIPTION  *******************
*******************************************************************************/
const SYS_PERIPH_TYPE_T SYSIRQ[SYSIRQ_NUMBER+1] =
{ 
/*	{Peripheral Type,     Interrupt Number[ch],Reserved,Reserved}                
 *	{                 {    ch0,    ch1,    ch2}, LdmaRx,  LdmaTx},	Periph 
 */
	{EPC_E,           { UNUSED, UNUSED, UNUSED}, UNUSED, UNUSED},	/*< EPC0 */
   	{SFM_E,           { UNUSED, UNUSED, UNUSED}, UNUSED, UNUSED},	/*< SFM0 */   
   	{CM_E,            {      1, UNUSED, UNUSED}, UNUSED, UNUSED},	/*< CM0  */   
   	{UNUSED_SYS_PERIPH_TYPE_E ,{UNUSED, UNUSED,  UNUSED},UNUSED, UNUSED},
   	{IRC_E,           { UNUSED, UNUSED, UNUSED}, UNUSED, UNUSED},	/*< IRC  */
   	{UNUSED_SYS_PERIPH_TYPE_E ,{UNUSED, UNUSED,  UNUSED},UNUSED, UNUSED},
   	{LDMAC_E,         {     28, UNUSED, UNUSED}, UNUSED, UNUSED},	/*< LDMAC*/
};

/*******************************************************************************
*******************   EXTERNAL INTERRUPT DESCRIPTION   *************************
*******************************************************************************/
const EXTIRQ_TYPE_T EXTIRQ[EXTIRQ_NUMBER+1] =
{ 
/*	{Ext IRQ Type, Interrupt Number}-*  i - ExtIrq#*/   
   	{IRQ0_E,                40},    /*  0  */
   	{IRQ1_E,                41},    /*  1  */
   	{IRQ2_E,                42},    /*  2  */
   	{IRQ3_E,                43},    /*  3  */
   	{IRQ4_E,                44},    /*  4  */
   	{IRQ5_E,                45},    /*  5  */
   	{IRQ6_E,                46},    /*  6  */
   	{IRQ7_E,                47},    /*  7  */
   	{IRQ8_E,                48},    /*  8  */
   	{IRQ9_E,                49},    /*  9  */
   	{IRQ10_E,               50},    /*  10 */
   	{IRQ11_E,               51},    /*  11 */
   	{IRQ12_E,               52},    /*  12 */
   	{IRQ13_E,               53},    /*  13 */
   	{IRQ14_E,               54},    /*  14 */
   	{IRQ15_E,               55},    /*  15 */
   	{IRQ16_E,               56},    /*  16 */
   	{IRQ17_E,               57},    /*  17 */
   	{IRQ18_E,               58},    /*  18 */
   	{IRQ19_E,               59},    /*  19 */
   	{UNUSED_IRQ_TYPE_E, UNUSED}
};
                                                 
/*******************************************************************************
*******************   SOFTWARE INTERRUPT DESCRIPTION   *************************
***************   CAUTION: This is not a SWI instruction   *********************
*******************************************************************************/
const SWIRQ_TYPE_T SWIRQ[SWIRQ_NUMBER+1] =
{ 
/* 	{Sw IRQ Type, Interrupt Number} -*  i - SwIrq#*/
	{SWIRQ1_E,              22},    /*  1  */
   	{SWIRQ2_E,              39},    /*  2  */
   	{SWIRQ3_E,              60},    /*  3  */
   	{SWIRQ4_E,              61},    /*  4  */
   	{SWIRQ5_E,              62},    /*  5  */
   	{SWIRQ6_E,              63},    /*  6  */
   	{UNUSED_IRQ_TYPE_E, UNUSED}
};
        
/*******************************************************************************
***********************   PIO BLOCK MASK DESCRIPTION   *************************
*******************************************************************************/
const PIO_BLOCK_TYPE_T PIO_BLOCK_MASK[PIO_BLOCK_NUMBER+1] =
{
	{(CSP_PIO_PTR)PIO0_BASE_ADDRESS, 0x007FFFFF},
    {(CSP_PIO_PTR)PIO1_BASE_ADDRESS, 0xFFFFFFFF},
    {(CSP_PIO_PTR)PIO2_BASE_ADDRESS, 0x00FFFFFF},
    {(CSP_PIO_PTR)PIO3_BASE_ADDRESS, 0xFFFFFFFF},
    {(CSP_PIO_PTR)UNUSED_ADDRESS,    0x00000000}
};
