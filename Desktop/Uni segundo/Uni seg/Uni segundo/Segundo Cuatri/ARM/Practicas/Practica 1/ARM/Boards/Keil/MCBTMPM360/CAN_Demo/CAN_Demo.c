/*----------------------------------------------------------------------------
 * Name:    CAN_Demo.c
 * Purpose: CAN Demonstration
 * Note(s):
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2010 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <CAN.h>                        /* CAN functions & defines            */
#include "BTN.h"                        /* Buttons functions prototypes       */
#include "LED.h"                        /* LED functions prototypes           */
#include "ADC.h"                        /* ADC functions prototypes           */
#include "TMPM364.h"

unsigned int Clock100ms;


void SysTick_Handler (void) {           /* SysTick Interrupt Handler (1ms)    */
  static int SysTickCnt;

  if (SysTickCnt++ >= 99) {
    SysTickCnt = 0;
    Clock100ms = 1;
  }
}

int main (void)  {
  /* Initialize message  = { ID, {data[0] .. data[7]}, LEN, CHANNEL, FORMAT, TYPE } */
  CAN_msg msg_send       = { 33, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   
                              1, 2, STANDARD_FORMAT, DATA_FRAME };
  CAN_msg msg_rece;

  SysTick_Config(SystemCoreClock/1000); /* Generate interrupt every 1 ms      */

  BTN_Init();                           /* Initialize buttons                 */
  LED_Init();                           /* Initialize LEDs                    */
  ADC_Init();                           /* Initialize ADC                     */

  can_init (1, 500000);                 /* CAN controller 1 init, 500 kbit/s  */

  can_rx_object (1, 1,  33, DATA_TYPE | STANDARD_TYPE); /* Enable reception   */
                                        /* of message on controller 1, channel*/
                                        /* 1, data frame with standard id 33  */

  if (BTN_Get() & (1 << 0)) {           /* If PJ4 button pressed              */
    can_testmode(1, 1);                 /* Activate loopback                  */
  }

  can_start (1);                        /* Start controller 1                 */

  while (1) {                           /* Endless loop                       */
    if (Clock100ms) {
      Clock100ms = 0;
      msg_send.data[0] = ADC_Get() >> 2;/* Data[0] field = analog value from  */
                                        /* potentiometer                      */

      can_send (1, &msg_send);          /* Send msg_send on controller 1      */
    }

    if (!can_receive (1,1, &msg_rece)) {/* If message received                */
      LED_Out(msg_rece.data[0] >> 4);   /* Display high 4 bits of received    */
                                        /* value on 4 LEDs                    */
    }
  }
}
