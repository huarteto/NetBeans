/*----------------------------------------------------------------------------
 *      RL-ARM - RTX
 *----------------------------------------------------------------------------
 *      Name:    BLINKY.C
 *      Purpose: RTX example program
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2009 KEIL - An ARM Company. All rights reserved. 
 *---------------------------------------------------------------------------*/

#include <RTL.h>
#include <73x_lib.h>


OS_TID t_ledA;                        /* assigned task id of task: led_a */
OS_TID t_ledB;                        /* assigned task id of task: led_b */
OS_TID t_ledC;                        /* assigned task id of task: led_c */

GPIO_InitTypeDef  GPIO2_InitStructure;

/*----------------------------------------------------------------------------
 *        Task 1 'ledA': led A output
 *---------------------------------------------------------------------------*/
__task void ledA  (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
     GPIO_BitWrite(GPIO2, GPIO_PIN_4, Bit_RESET);
     GPIO_BitWrite(GPIO2, GPIO_PIN_6, Bit_SET);
     os_dly_wait(20);
     GPIO_BitWrite(GPIO2, GPIO_PIN_6, Bit_RESET);
     GPIO_BitWrite(GPIO2, GPIO_PIN_8, Bit_SET);
	 os_dly_wait(20);
	 os_evt_set(0x0001,t_ledB);
     os_dly_wait(100);
  }
}

/*----------------------------------------------------------------------------
 *        Task 2 'ledB': led B output
 *---------------------------------------------------------------------------*/
__task void ledB (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
   	GPIO_BitWrite(GPIO2, GPIO_PIN_8, Bit_RESET);
    GPIO_BitWrite(GPIO2, GPIO_PIN_10, Bit_SET);
    os_dly_wait(20);
	GPIO_BitWrite(GPIO2, GPIO_PIN_10, Bit_RESET);
    GPIO_BitWrite(GPIO2, GPIO_PIN_12, Bit_SET); 
	os_dly_wait(20);
	os_evt_set(0x0001,t_ledC);
 	os_dly_wait(70);
  }
}

/*----------------------------------------------------------------------------
 *        Task 3 'ledC': led C output
 *---------------------------------------------------------------------------*/
__task void ledC (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
   	GPIO_BitWrite(GPIO2, GPIO_PIN_12, Bit_RESET);
    GPIO_BitWrite(GPIO2, GPIO_PIN_15, Bit_SET); 
	os_dly_wait(20);
	GPIO_BitWrite(GPIO2, GPIO_PIN_15, Bit_RESET); 
	GPIO_BitWrite(GPIO2, GPIO_PIN_4, Bit_SET);
	os_dly_wait(20);
	os_evt_set(0x0001,t_ledA);
  	os_dly_wait(50);
  }
}


/*----------------------------------------------------------------------------
 *       Task 4 'init': Initialize
 *---------------------------------------------------------------------------*/
__task void init (void) {
 
 /* GPIO2 Clock Enable */
  CFG_PeripheralClockConfig(CFG_CLK_GPIO2 , ENABLE);            
  /* GPIO2 Configuration */ 
  GPIO2_InitStructure.GPIO_Mode = GPIO_Mode_OUT_PP;
  GPIO2_InitStructure.GPIO_Pins = GPIO_PIN_4 | GPIO_PIN_6 | GPIO_PIN_8 | GPIO_PIN_10 | GPIO_PIN_12 | GPIO_PIN_15;
  GPIO_Init (GPIO2, &GPIO2_InitStructure);
  

  t_ledA = os_tsk_create (ledA, 0);  /* start task ledA                */
  t_ledB = os_tsk_create (ledB, 0);  /* start task ledB                */
  t_ledC = os_tsk_create (ledC, 0);  /* start task ledC                */
  os_evt_set(0x0001,t_ledA);         /* send event signal to ledA		*/  
  os_tsk_delete_self ();
}

/*----------------------------------------------------------------------------
 *        Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/
int main (void) {
  os_sys_init (init);                    /* Initialize RTX and start init    */
}

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/

