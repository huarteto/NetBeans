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


// I/O mapping of the 16 Red LEDs
struct {
  GPIO_TypeDef* gpio;
  u32           bit;
} LED_Map[16] = {
  { GPIO0,  3 },
  { GPIO0,  2 },
  { GPIO0,  1 },
  { GPIO0,  0 },
  { GPIO0, 12 },
  { GPIO1, 15 },
  { GPIO2,  9 },
  { GPIO2, 10 },
  { GPIO2, 11 },
  { GPIO2, 12 },
  { GPIO2, 13 },
  { GPIO2, 14 },
  { GPIO2, 15 },
  { GPIO1,  6 },
  { GPIO1,  5 },
  { GPIO1,  4 }
};

void LED_Init(void) {
  u32 i;
  u16 uMask;

  // I/O Ports configured as Output (Push-pull)
  for (i = 0; i < 16; i++) {
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


extern vs16 ADVal;
extern vu32 TimeVal;

extern __irq void T3TIMI_IRQHandler(void);
extern __irq void ADC12_IRQHandler (void);


int main (void) {
  u32 delay;
  u32 time;

  RCCU_MCLKConfig(RCCU_DEFAULT);                    // MCLK = RCLK
  RCCU_FCLKConfig(RCCU_RCLK_2);                     // FCLK = RCLK/2
  RCCU_PCLKConfig(RCCU_RCLK_4);                     // PCLK = RCLK/4
  RCCU_PLL1Config(RCCU_PLL1_Mul_12, RCCU_Div_2);    // 48MHz PLL @ 16MHz XTAL

  while (RCCU_FlagStatus(RCCU_PLL1_LOCK) == RESET); // Wait for PLL to Lock
  RCCU_RCLKSourceConfig(RCCU_PLL1_Output);          // Select PLL for RCLK

  GPIO_Config(GPIO1, 0x0001, GPIO_HI_AIN_TRI);      // P1.0: Analog Input
  GPIO_Config(GPIO1, 0x0100, GPIO_IN_TRI_CMOS);     // P1.8: Input (SEL Button)
  GPIO_Config(GPIO1, 0x0004, GPIO_OUT_PP);          // P1.2: Output (R/G LED)

  LED_Init();                                       // LED Initialization

  // A/D Converter Configuration: Read Potenciometer Position
  ADC12_Init();                                     // A/D Initialization
  ADC12_PrescalerConfig(500);                       // 500Hz Sampling Rate
  ADC12_ModeConfig(ADC12_SINGLE);                   // Single Channel
  ADC12_ChannelSelect(ADC12_CHANNEL0);              // Select Channel 0
  ADC12_ITConfig(ENABLE);                           // Enable Interrupt
  ADC12_ConversionStart();                          // Start A/D Conversion

  // Configure and enable IRQ for A/D Converter
  EIC->SIR[ADC_IRQChannel] = ((u32)ADC12_IRQHandler << 16);
  EIC_IRQChannelConfig(ADC_IRQChannel, ENABLE);
  EIC_IRQChannelPriorityConfig(ADC_IRQChannel, 2);

  // Timer 3 Configuration: Periodic Interrupts at aprox. 10ms
  TIM_Init (TIM3);                                  // Initialization
  TIM_PrescalerConfig (TIM3, 1);                    // Prescaler
  TIM_ITConfig (TIM3, TIM_TO_IT, ENABLE);           // Enable Overflow Interrupt
  TIM_CounterConfig (TIM3,TIM_START);               // Start Timer

  // Configure and enable IRQ for Timer 3
  EIC->IVR = (u32)T3TIMI_IRQHandler;
  EIC->SIR[T3TIMI_IRQChannel] = ((u32)T3TIMI_IRQHandler << 16);
  EIC_IRQChannelConfig(T3TIMI_IRQChannel, ENABLE);
  EIC_IRQChannelPriorityConfig(T3TIMI_IRQChannel, 1);

  EIC_IRQConfig(ENABLE);                            // Configure IRQ

  while (1) {                                       // Loop forever
    GPIO1->PD ^= 0x0004;                            // Toggle P1.2 (R/G LED)
    delay = (GPIO1->PD & 0x0100) ? 50 : 25;         // P1.8 (SEL Button)
    time  = TimeVal;
    while ((TimeVal - time) < delay);               // Wait for 500ms/250ms
  }
}
