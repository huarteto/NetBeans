/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <71x_lib.H>
#include <stdio.h>


#define UART0_Rx_Pin (0x0001<<8)   // TQFP 64: pin N° 63 , TQFP 144 pin N° 143
#define UART0_Tx_Pin (0x0001<<9)   // TQFP 64: pin N° 64 , TQFP 144 pin N° 144

#define UART1_Rx_Pin (0x0001<<10)  // TQFP 64: pin N° 1  , TQFP 144 pin N° 1
#define UART1_Tx_Pin (0x0001<<11)  // TQFP 64: pin N° 2  , TQFP 144 pin N° 3

#define UART2_Rx_Pin (0x0001<<13)  // TQFP 64: pin N° 5  , TQFP 144 pin N° 9
#define UART2_Tx_Pin (0x0001<<14)  // TQFP 64: pin N° 6  , TQFP 144 pin N° 10

#define UART3_Rx_Pin (0x0001<<1)   // TQFP 64: pin N° 52 , TQFP 144 pin N° 123
#define UART3_Tx_Pin (0x0001<<0)   // TQFP 64: pin N° 53 , TQFP 144 pin N° 124

#define Use_UART0
//#define Use_UART1
//#define Use_UART2
//#define Use_UART3

   #ifdef Use_UART0
     #define  UARTX  UART0
     #define  UARTX_Rx_Pin  UART0_Rx_Pin
     #define  UARTX_Tx_Pin  UART0_Tx_Pin
     #define  UARTX_Periph  UART0_Periph
   #endif /* Use_UART0 */

   #ifdef Use_UART1
     #define  UARTX  UART1
     #define  UARTX_Rx_Pin  UART1_Rx_Pin
     #define  UARTX_Tx_Pin  UART1_Tx_Pin
     #define  UARTX_Periph  UART1_Periph
   #endif /* Use_UART1 */

   #ifdef Use_UART2
     #define  UARTX  UART2
     #define  UARTX_Rx_Pin  UART2_Rx_Pin
     #define  UARTX_Tx_Pin  UART2_Tx_Pin
     #define  UARTX_Periph  UART2_Periph
   #endif /* Use_UART2 */

   #ifdef Use_UART3
     #define  UARTX  UART3
     #define  UARTX_Rx_Pin  UART3_Rx_Pin
     #define  UARTX_Tx_Pin  UART3_Tx_Pin
     #define  UARTX_Periph  UART3_Periph
   #endif /* Use_UART3 */


// I/O mapping of the 8 LEDs
struct {
  GPIO_TypeDef* gpio;
  u32           bit;
} LED_Map[8] = {
  { GPIO2,  8 },
  { GPIO2,  9 },
  { GPIO2, 10 },
  { GPIO2, 11 },
  { GPIO2, 12 },
  { GPIO2, 13 },
  { GPIO2, 14 },
  { GPIO2, 15 },
};

void LED_Init(void) {
  u32 i;
  u16 uMask;

  // I/O Ports configured as Output (Push-pull)
  for (i = 0; i < 8; i++) {
    uMask = 1 << LED_Map[i].bit;
    LED_Map[i].gpio->PC0 |= uMask;
    LED_Map[i].gpio->PC1 &=~uMask;
    LED_Map[i].gpio->PC2 |= uMask;
    LED_Map[i].gpio->PD  &=~uMask;
  }
}

void LED_On(u32 num) {
  LED_Map[num].gpio->PD |=  (1 << LED_Map[num].bit);
}

void LED_Off(u32 num) {
  LED_Map[num].gpio->PD &= ~(1 << LED_Map[num].bit);
}


void delay (void) {
  u32 dly_cnt = 1000000;

  while(dly_cnt) {
    dly_cnt--;
  } 
}


int main (void) {
  u32 n;

  RCCU_MCLKConfig(RCCU_DEFAULT);                    // MCLK = RCLK
  RCCU_FCLKConfig(RCCU_RCLK_2);                     // FCLK = RCLK/2
  RCCU_PCLKConfig(RCCU_RCLK_4);                     // PCLK = RCLK/4
  RCCU_PLL1Config(RCCU_PLL1_Mul_12, RCCU_Div_2);    // 48MHz PLL @ 16MHz XTAL

  while (RCCU_FlagStatus(RCCU_PLL1_LOCK) == RESET); // Wait for PLL to Lock
  RCCU_RCLKSourceConfig(RCCU_PLL1_Output);          // Select PLL for RCLK

  LED_Init();                                       // LED Initialization

  // Configure the GPIO pins
  GPIO_Config(GPIO0, UARTX_Tx_Pin, GPIO_AF_PP);
  GPIO_Config(GPIO0, UARTX_Rx_Pin, GPIO_IN_TRI_CMOS);

  // Configure the UART X
  UART_OnOffConfig(UARTX, ENABLE);                  // Turn UARTX On
  UART_FifoConfig (UARTX, DISABLE);                 // Disable FIFOs
  UART_FifoReset  (UARTX , UART_RxFIFO);            // Reset the UART_RxFIFO
  UART_FifoReset  (UARTX , UART_TxFIFO);            // Reset the UART_TxFIFO
  UART_LoopBackConfig(UARTX , DISABLE);             // Disable Loop Back
  // 9600 Bps, No parity, 8 data bits, 1 stop bit
  UART_Config(UARTX, 9600, UART_NO_PARITY, UART_1_StopBits, UARTM_8D);
  UART_RxConfig(UARTX ,ENABLE);                     // Enable Rx

  while (1) {                                       // Loop forever
    for (n = 0; n < 8; n++)  {
      LED_On(n);
      delay(); 
    }
    for (n = 0; n < 8; n++)  {
      LED_Off(n);
      delay(); 
    }   
    printf("Hello World\n");
  }
}
