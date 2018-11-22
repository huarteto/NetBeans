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
#include <73x_lib.H>


OS_TID t_phaseA;                        /* assigned task id of task: phase_a */
OS_TID t_phaseB;                        /* assigned task id of task: phase_b */
OS_TID t_phaseC;                        /* assigned task id of task: phase_c */
OS_TID t_phaseD;                        /* assigned task id of task: phase_d */
OS_TID t_clock;                         /* assigned task id of task: clock   */

PRCCU_InitTypeDef PRCCU_InitStruct;     /* Declare PRCCU structure           */
CMU_InitTypeDef   CMU_InitStruct;       /* Declare CMU   structure           */

#define LED_A   0x0040
#define LED_B   0x0004
#define LED_C   0x0008
#define LED_D   0x0010
#define LED_CLK 0x0080

void LED_Init (void) {
  /* I/O Ports configured as Output (Push-pull) */
  ((GPIO_TypeDef *)GPIO0)->PC0 |= 0x03FF;
  ((GPIO_TypeDef *)GPIO0)->PC1 &=~0x03FF;
  ((GPIO_TypeDef *)GPIO0)->PC2 |= 0X03FF;
  ((GPIO_TypeDef *)GPIO0)->PD  |= 0x02FF;
}

void LED_On (U32 num) {
  ((GPIO_TypeDef *)GPIO0)->PD &= ~num;
}

void LED_Off (U32 num) {
  ((GPIO_TypeDef *)GPIO0)->PD |= num;
}

/*----------------------------------------------------------------------------
 *        Function 'signal_func' called from multiple tasks
 *---------------------------------------------------------------------------*/
void signal_func (OS_TID task)  {
  os_evt_set (0x0100, t_clock);          /* send event signal to clock task  */
  os_dly_wait (50);                      /* delay 50 clock ticks             */
  os_evt_set (0x0100, t_clock);          /* send event signal to clock task  */
  os_dly_wait (50);                      /* delay 50 clock ticks             */
  os_evt_set (0x0001, task);             /* send event to task 'task'        */
  os_dly_wait (50);                      /* delay 50 clock ticks             */
}

/*----------------------------------------------------------------------------
 *        Task 1 'phaseA': Phase A output
 *---------------------------------------------------------------------------*/
__task void phaseA (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    LED_On (LED_A);
    signal_func (t_phaseB);              /* call common signal function      */
    LED_Off(LED_A);
  }
}

/*----------------------------------------------------------------------------
 *        Task 2 'phaseB': Phase B output
 *---------------------------------------------------------------------------*/
__task void phaseB (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    LED_On (LED_B);
    signal_func (t_phaseC);              /* call common signal function      */
    LED_Off(LED_B);
  }
}

/*----------------------------------------------------------------------------
 *        Task 3 'phaseC': Phase C output
 *---------------------------------------------------------------------------*/
__task void phaseC (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    LED_On (LED_C);
    signal_func (t_phaseD);              /* call common signal function      */
    LED_Off(LED_C);
  }
}

/*----------------------------------------------------------------------------
 *        Task 4 'phaseD': Phase D output
 *---------------------------------------------------------------------------*/
__task void phaseD (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    LED_On (LED_D);
    signal_func (t_phaseA);              /* call common signal function      */
    LED_Off(LED_D);
  }
}

/*----------------------------------------------------------------------------
 *        Task 5 'clock': Signal Clock
 *---------------------------------------------------------------------------*/
__task void clock (void) {
  for (;;) {
    os_evt_wait_and (0x0100, 0xffff);    /* wait for an event flag 0x0100    */
    LED_On (LED_CLK);
    os_dly_wait (8);                     /* delay 8 clock ticks              */
    LED_Off(LED_CLK);
  }
}

/*----------------------------------------------------------------------------
 *        Task 6 'init': Initialize
 *---------------------------------------------------------------------------*/
__task void init (void) {

  /* Power, Reset and Clock Control Unit (PRCCU) Setup                       */
  /* Setup clock to frequency of oscillator                                  */
  PRCCU_DeInit();
  PRCCU_StructInit(&PRCCU_InitStruct);  
  PRCCU_InitStruct.PRCCU_MCLKSRC_SRC    = PRCCU_MCLKSRC_PLL;
  PRCCU_InitStruct.PRCCU_DIV2           = ENABLE;
  PRCCU_InitStruct.PRCCU_PLLMUL         = PRCCU_PLLMUL_16;
  PRCCU_InitStruct.PRCCU_PLLDIV         = PRCCU_PLLDIV_2;    // MCLK = 32MHz
  PRCCU_Init(&PRCCU_InitStruct);

  /* Clock Monitor Unit (CMU) Setup                                          */
  /* Setup CMU to use oscillator                                             */
  CMU_DeInit();
  CMU_StructInit(&CMU_InitStruct);
  CMU_InitStruct.CMU_CKSEL0             = CMU_CKSEL0_CKOSC;
  CMU_Init(&CMU_InitStruct);

  LED_Init();                                       // LED Initialization

  t_phaseA = os_tsk_create (phaseA, 0);  /* start task phaseA                */
  t_phaseB = os_tsk_create (phaseB, 0);  /* start task phaseB                */
  t_phaseC = os_tsk_create (phaseC, 0);  /* start task phaseC                */
  t_phaseD = os_tsk_create (phaseD, 0);  /* start task phaseD                */
  t_clock  = os_tsk_create (clock, 0);   /* start task clock                 */
  os_evt_set (0x0001, t_phaseA);         /* send signal event to task phaseA */
  os_tsk_delete_self ();
}

/*----------------------------------------------------------------------------
 *        Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/
int main (void) {
  CFG_PeripheralClockConfig(CFG_CLK_GPIO0, ENABLE);
  CFG_PeripheralClockConfig(CFG_CLK_EIC, ENABLE);

  os_sys_init (init);                    /* Initialize RTX and start init    */
}

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/

