/*----------------------------------------------------------------------------
 *      Name:    SERIAL.C
 *      Purpose: Low Level Serial Routines
 *      Version: V1.10
 *----------------------------------------------------------------------------
 *      This file is part of the uVision/ARM development tools.
 *      This software may only be used under the terms of a valid, current,
 *      end user licence from KEIL for a compatible version of KEIL software
 *      development tools. Nothing else gives you the right to use it.
 *
 *      Copyright (c) 2005-2007 Keil Software.
 *---------------------------------------------------------------------------*/

#include <91x_lib.h> 

void com_baudrate (unsigned int baudrate)
{
  unsigned long      BRDi = 0, BRDf = 0;
  unsigned long long UARTClk = 48000000;

  if ((SCU->CLKCNTR & SCU_BRCLK_Div2) != SCU_BRCLK_Div2) { 
    UARTClk /= 2;							   /* set UART Clock acc. BRSEL  */
  }

  /* baudrate calculation */
  BRDi = ((100 * UARTClk) / (16 * baudrate));  /* calculate the integer part */
  BRDf = BRDi - (100 * (BRDi / 100));          /* calculate the fractal part */

  UART0->CR  &= 0xFFFE;                        /* disable UART               */

  UART0->IBRD = BRDi / 100;				       /* set the integer part       */
  UART0->FBRD = ((((BRDf * 64) + 50) / 100));  /* set the fractal part       */

  UART0->LCR &= 0xFFFF;                        /* to accept baudrate         */
  UART0->CR  |= 0x0001;  			           /* enable UART                */	 
}

void init_serial (void)  {                 /* Initialize Serial Interface     */
 /* UART0 Setup.....................................................*/	
  SCU->GPIOOUT[3] = 0x0008;     /* P3.1 output UART0 TX */
  SCU->GPIOIN[3]  = 0x01;       /* P3.0 input  UART0 RX */

  UART0->LCR  =   UART_WordLength_8D              /* 8 bits              */
                | UART_Parity_No                  /* no Parity           */
				| UART_HardwareFlowControl_None   /* no flow control     */
				& UART_StopBits_1                 /* 1 Stop bit          */
				& UART_FIFO_Disable;              /* FIFO disabled       */

  UART0->CR   =   UART_Mode_Rx 					  /* Receive Enable      */
                | UART_Mode_Tx					  /* Transmit Enable     */
                | 0x0001;                         /* UART Enable         */

  com_baudrate (115200);                          /* 115200 Baud         */
}


/* implementation of putchar (also used by printf function to output data)    */
int sendchar (int ch)  {                   /* Write character to Serial Port  */

  if (ch == '\n')  {
    while ((UART0->FR & 0x0020));			 /* TX Fifo Full */
    UART0->DR = 0x0D;                        /* output CR */
  }
  while ((UART0->FR & 0x0020));
  return (UART0->DR = ch);
#if 0
   if (ch == '\n')  {
      while (!(UART0->FR & 0x0080));        /* NOT TX Fifo Empty */   
      UART0->DR = '\r';               
   }
   while (!(UART0->FR & 0x0080));      
   return(UART0->DR = ch); 
#endif              
}


int getkey (void)  {                       /* Read character from Serial Port */

  while ((UART0->FR & 0x10));			   /* RX Fifo Empty */
  return (UART0->DR);
}
