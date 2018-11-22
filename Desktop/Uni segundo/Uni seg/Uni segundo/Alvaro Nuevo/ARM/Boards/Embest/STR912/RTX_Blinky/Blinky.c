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
#include <91x_lib.H>

OS_TID t_phaseA;                        /* assigned task id of task: phase_a */
OS_TID t_phaseB;                        /* assigned task id of task: phase_b */
OS_TID t_phaseC;                        /* assigned task id of task: phase_c */
OS_TID t_phaseD;                        /* assigned task id of task: phase_d */
OS_TID t_clock;                         /* assigned task id of task: clock   */
OS_TID t_lcd;                           /* assigned task id of task: lcd     */

#define LED_A   0x01
#define LED_B   0x02
#define LED_C   0x04
#define LED_D   0x08
#define LED_CLK 0x80

GPIO_InitTypeDef GPIO_InitStructure;


/*----------------------------------------------------------------------------
 *        Task 1 'phaseA': Phase A output
 *---------------------------------------------------------------------------*/
__task void phaseA (void) {
  for (;;) {

    GPIO_WriteBit(GPIO6, GPIO_Pin_6, Bit_RESET);
    GPIO_WriteBit(GPIO0, GPIO_Pin_1, Bit_SET);
    GPIO_WriteBit(GPIO3, GPIO_Pin_7, Bit_SET);
    GPIO_WriteBit(GPIO9, GPIO_Pin_6, Bit_SET);

    os_dly_wait(5);

    GPIO_WriteBit(GPIO6, GPIO_Pin_6, Bit_RESET);
    GPIO_WriteBit(GPIO0, GPIO_Pin_1, Bit_SET);
    GPIO_WriteBit(GPIO3, GPIO_Pin_7, Bit_SET);
    GPIO_WriteBit(GPIO9, GPIO_Pin_6, Bit_SET);
  }
}

/*----------------------------------------------------------------------------
 *        Task 2 'phaseB': Phase B output
 *---------------------------------------------------------------------------*/
__task void phaseB (void) {
  for (;;) {
    GPIO_WriteBit(GPIO6, GPIO_Pin_6, Bit_SET);
    GPIO_WriteBit(GPIO0, GPIO_Pin_1, Bit_SET);
    GPIO_WriteBit(GPIO3, GPIO_Pin_7, Bit_SET);
    GPIO_WriteBit(GPIO9, GPIO_Pin_6, Bit_RESET);

    os_dly_wait(6);

    GPIO_WriteBit(GPIO9, GPIO_Pin_6, Bit_SET);
    GPIO_WriteBit(GPIO6, GPIO_Pin_6, Bit_SET);
    GPIO_WriteBit(GPIO0, GPIO_Pin_1, Bit_SET);
    GPIO_WriteBit(GPIO3, GPIO_Pin_7, Bit_SET);
    GPIO_WriteBit(GPIO9, GPIO_Pin_6, Bit_RESET);
  }
}

/*----------------------------------------------------------------------------
 *        Task 3 'phaseC': Phase C output
 *---------------------------------------------------------------------------*/
__task void phaseC (void) {
  for (;;) {

    GPIO_WriteBit(GPIO6, GPIO_Pin_6, Bit_SET);
    GPIO_WriteBit(GPIO0, GPIO_Pin_1, Bit_SET);
    GPIO_WriteBit(GPIO3, GPIO_Pin_7, Bit_RESET);
    GPIO_WriteBit(GPIO9, GPIO_Pin_6, Bit_SET);

    os_dly_wait(7);

    GPIO_WriteBit(GPIO3, GPIO_Pin_7, Bit_SET);
    GPIO_WriteBit(GPIO6, GPIO_Pin_6, Bit_SET);
    GPIO_WriteBit(GPIO0, GPIO_Pin_1, Bit_SET);
    GPIO_WriteBit(GPIO3, GPIO_Pin_7, Bit_RESET);
    GPIO_WriteBit(GPIO9, GPIO_Pin_6, Bit_SET);
  }
}

/*----------------------------------------------------------------------------
 *        Task 4 'phaseD': Phase D output
 *---------------------------------------------------------------------------*/
__task void phaseD (void) {
  for (;;) {
    GPIO_WriteBit(GPIO6, GPIO_Pin_6, Bit_SET);
    GPIO_WriteBit(GPIO0, GPIO_Pin_1, Bit_RESET);
    GPIO_WriteBit(GPIO3, GPIO_Pin_7, Bit_SET);
    GPIO_WriteBit(GPIO9, GPIO_Pin_6, Bit_SET);

    os_dly_wait(8);

    GPIO_WriteBit(GPIO0, GPIO_Pin_1, Bit_SET);
    GPIO_WriteBit(GPIO6, GPIO_Pin_6, Bit_SET);
    GPIO_WriteBit(GPIO0, GPIO_Pin_1, Bit_RESET);
    GPIO_WriteBit(GPIO3, GPIO_Pin_7, Bit_SET);
    GPIO_WriteBit(GPIO9, GPIO_Pin_6, Bit_SET);
  }
}




void SCU_Configuration(void)
{
  /* Enable the __GPIO */
  SCU_APBPeriphClockConfig(__GPIO0 ,ENABLE);
  SCU_APBPeriphClockConfig(__GPIO3 ,ENABLE);
  SCU_APBPeriphClockConfig(__GPIO6 ,ENABLE);
  SCU_APBPeriphClockConfig(__GPIO9 ,ENABLE);

}






/*----------------------------------------------------------------------------
 *        Task 5 'init': Initialize
 *---------------------------------------------------------------------------*/
__task void init (void) {

  SCU_Configuration();

  GPIO_DeInit(GPIO0);
  GPIO_DeInit(GPIO3);
  GPIO_DeInit(GPIO6);
  GPIO_DeInit(GPIO9);

  GPIO_InitStructure.GPIO_Direction = GPIO_PinOutput;

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_Init (GPIO0, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_Init (GPIO3, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_Init (GPIO6, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_Init (GPIO9, &GPIO_InitStructure);


  GPIO_InitStructure.GPIO_Type = GPIO_Type_PushPull;

  t_phaseA = os_tsk_create (phaseA, 5);  /* start task phaseA                */
  t_phaseB = os_tsk_create (phaseB, 4);  /* start task phaseB                */
  t_phaseC = os_tsk_create (phaseC, 3);  /* start task phaseC                */
  t_phaseD = os_tsk_create (phaseD, 2);  /* start task phaseD                */
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

