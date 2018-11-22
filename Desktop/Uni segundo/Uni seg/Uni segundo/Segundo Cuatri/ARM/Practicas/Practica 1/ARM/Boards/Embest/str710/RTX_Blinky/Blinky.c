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
#include <71x_lib.H>


OS_TID t_ledA;                        /* assigned task id of task: led_a */
OS_TID t_ledB;                        /* assigned task id of task: led_b */
OS_TID t_ledC;                        /* assigned task id of task: led_c */



/*----------------------------------------------------------------------------
 *        Task 1 'ledA': led A output
 *---------------------------------------------------------------------------*/
__task void ledA  (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    GPIO_BitWrite(GPIO2, 0xa, ~GPIO_BitRead(GPIO2, 0xa));
    os_dly_wait(20);
    GPIO_BitWrite(GPIO2, 0xa, ~GPIO_BitRead(GPIO2, 0xa));
	os_evt_set(0x0001,t_ledB);
    
  }
}

/*----------------------------------------------------------------------------
 *        Task 2 'ledB': led B output
 *---------------------------------------------------------------------------*/
__task void ledB (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
   	GPIO_BitWrite(GPIO2, 0xc, ~GPIO_BitRead(GPIO2, 0xc)); 
    os_dly_wait(20);
	GPIO_BitWrite(GPIO2, 0xc, ~GPIO_BitRead(GPIO2, 0xc)); 
	os_evt_set(0x0001,t_ledC);
  }
}

/*----------------------------------------------------------------------------
 *        Task 3 'ledC': led C output
 *---------------------------------------------------------------------------*/
__task void ledC (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
   	 
	GPIO_BitWrite(GPIO2, 0xe, ~GPIO_BitRead(GPIO2, 0xe)); 
    os_dly_wait(20);
	GPIO_BitWrite(GPIO2, 0xe, ~GPIO_BitRead(GPIO2, 0xe)); 
	os_evt_set(0x0001,t_ledA);
  }
}


/*----------------------------------------------------------------------------
 *        Task 4 'init': Initialize
 *---------------------------------------------------------------------------*/
__task void init (void) {

  RCCU_MCLKConfig(RCCU_DEFAULT);                    // MCLK = RCLK
  RCCU_FCLKConfig(RCCU_RCLK_2);                     // FCLK = RCLK/2
  RCCU_PCLKConfig(RCCU_RCLK_4);                     // PCLK = RCLK/4
  RCCU_PLL1Config(RCCU_PLL1_Mul_12, RCCU_Div_2);    // 48MHz PLL @ 16MHz XTAL

  while (RCCU_FlagStatus(RCCU_PLL1_LOCK) == RESET); // Wait for PLL to Lock
  RCCU_RCLKSourceConfig(RCCU_PLL1_Output);          // Select PLL for RCLK

  /*   Configure P2.10,P2.12,P0.14 as Output Push-Pull */
  GPIO_Config(GPIO2, 0xFF00, GPIO_OUT_PP);                                    
  /*  turn off all  leds 					*/
  GPIO_WordWrite(GPIO2, 0x0);

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

