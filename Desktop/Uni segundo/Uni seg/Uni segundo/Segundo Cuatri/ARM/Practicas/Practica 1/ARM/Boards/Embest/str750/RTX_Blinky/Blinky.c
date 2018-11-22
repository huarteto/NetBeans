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
#include <75x_lib.H>                    /* STR750 definitions                */



#define LED_CLK     0x80

OS_TID t_ledA;                        /* assigned task id of task: led_a */
OS_TID t_ledB;                        /* assigned task id of task: led_b */
OS_TID t_ledC;                        /* assigned task id of task: led_c */
OS_TID t_ledD;                        /* assigned task id of task: led_d */
OS_TID t_ledE;                        /* assigned task id of task: led_e */
	

GPIO_InitTypeDef GPIO_InitStructure;
void MRCC_Configuration(void);

#define LED_On(led)     GPIO2->PD |=  (led)
#define LED_Off(led)    GPIO2->PD &= ~(led)


/*----------------------------------------------------------------------------
 *     Task 1 'ledA': ledA output
 *---------------------------------------------------------------------------*/
        

__task void ledA  (void) {
  for (;;) {
	GPIO_Write(GPIO2, 0x00000000);
    GPIO_Write(GPIO1, 0x00000400);

    os_dly_wait(15);
	GPIO_Write(GPIO2, 0x00000000);
    GPIO_Write(GPIO1, 0x00000400);

    
  }
}

/*----------------------------------------------------------------------------
 *              Task 2 'ledB': ledB output

 *---------------------------------------------------------------------------*/
__task void ledB (void) {
  for (;;) {
 	GPIO_Write(GPIO2, 0x00000200);
    GPIO_Write(GPIO1, 0x00000000);
    os_dly_wait(20);
	GPIO_Write(GPIO2, 0x00000200);
    GPIO_Write(GPIO1, 0x00000000);
  }
}

/*----------------------------------------------------------------------------
 *        Task 3 'ledC': led C output
 *---------------------------------------------------------------------------*/
__task void ledC (void) {
  for (;;) {
   	 
	GPIO_Write(GPIO2, 0x00000080);
    GPIO_Write(GPIO1, 0x00000000);
    os_dly_wait(30);
	GPIO_Write(GPIO2, 0x00000080);
    GPIO_Write(GPIO1, 0x00000000);
  }
}



__task void ledD (void) {
  for (;;) {
   	 
	GPIO_Write(GPIO2, 0x00000020);
    GPIO_Write(GPIO1, 0x00000000);
    os_dly_wait(35);
	GPIO_Write(GPIO2, 0x00000020);
    GPIO_Write(GPIO1, 0x00000000);
  }
}

__task void ledE (void) {
  for (;;) {
   	 
	GPIO_Write(GPIO2, 0x00000000);
    GPIO_Write(GPIO1, 0x00000040);

    os_dly_wait(40);
	GPIO_Write(GPIO2, 0x00000000);
    GPIO_Write(GPIO1, 0x00000040);

  }
}


/*----------------------------------------------------------------------------
 *        Task 6 'lcd': LCD Control task
 *---------------------------------------------------------------------------*/


void MRCC_Configuration(void)
{
  
  /* GPIO pins optimized for 3V3 operation */
  MRCC_IOVoltageRangeConfig(MRCC_IOVoltageRange_3V3); 
  
  /* Enable GPIO clock */
  MRCC_PeripheralClockConfig(MRCC_Peripheral_GPIO, ENABLE);
}


/*----------------------------------------------------------------------------
 *        Task 7 'init': Initialize
 *---------------------------------------------------------------------------*/
__task void init (void) {

   /* Configure the system clocks */
  MRCC_Configuration();

  /* GPIO Configuration --------------------------------------------------------*/
  /* Configure P2.07and P2.09and p2.05 sh-pull */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_9 | GPIO_Pin_7 | GPIO_Pin_5;
  GPIO_Init(GPIO2, &GPIO_InitStructure);

  /* Configure P1.10and p1.06+ as output push-pull */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10 | GPIO_Pin_6;
  GPIO_Init(GPIO1, &GPIO_InitStructure);

  
  /* Mask the unused pins */
  GPIO_PinMaskConfig(GPIO2, ~(GPIO_Pin_9 | GPIO_Pin_7 | GPIO_Pin_5), ENABLE);
  GPIO_PinMaskConfig(GPIO1, ~(GPIO_Pin_10 | GPIO_Pin_6), ENABLE);

  t_ledA = os_tsk_create (ledA, 5);  /* start task ledA                */
  t_ledB = os_tsk_create (ledB, 4);  /* start task ledB                */
  t_ledC = os_tsk_create (ledC, 3);  /* start task ledC                */
  t_ledD = os_tsk_create (ledD, 2);  /* start task ledD                */
  t_ledE = os_tsk_create (ledE, 1);  /* start task ledE                */


  os_tsk_delete_self ();
}

/*----------------------------------------------------------------------------
 *        Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/
int main (void) {
  U32 volatile start;

                                         /* Wait for debugger connection 0.3s*/
  for (start = 0; start < 1000000; start++) { ; }

  os_sys_init (init);                    /* Initialize RTX and start init    */
}

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/

