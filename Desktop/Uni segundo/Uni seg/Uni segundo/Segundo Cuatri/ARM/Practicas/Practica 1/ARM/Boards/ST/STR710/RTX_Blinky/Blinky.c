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


OS_TID t_phaseA;                        /* assigned task id of task: phase_a */
OS_TID t_phaseB;                        /* assigned task id of task: phase_b */
OS_TID t_phaseC;                        /* assigned task id of task: phase_c */
OS_TID t_phaseD;                        /* assigned task id of task: phase_d */
OS_TID t_clock;                         /* assigned task id of task: clock   */

#define LED_A   1
#define LED_B   2
#define LED_C   3
#define LED_D   4
#define LED_CLK 0


// I/O mapping of LEDs
struct {
  GPIO_TypeDef* gpio;
  u32           bit;
} LED_Map[5] = {
  { GPIO2, 10 },
  { GPIO2, 11 },
  { GPIO2, 12 },
  { GPIO2, 13 },
  { GPIO2, 14 }
};

void LED_Init(void) {
  u32 i;
  u16 uMask;

  // I/O Ports configured as Output (Push-pull)
  for (i = 0; i < 5; i++) {
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

  RCCU_MCLKConfig(RCCU_DEFAULT);                    // MCLK = RCLK
  RCCU_FCLKConfig(RCCU_RCLK_2);                     // FCLK = RCLK/2
  RCCU_PCLKConfig(RCCU_RCLK_4);                     // PCLK = RCLK/4
  RCCU_PLL1Config(RCCU_PLL1_Mul_12, RCCU_Div_2);    // 48MHz PLL @ 16MHz XTAL

  while (RCCU_FlagStatus(RCCU_PLL1_LOCK) == RESET); // Wait for PLL to Lock
  RCCU_RCLKSourceConfig(RCCU_PLL1_Output);          // Select PLL for RCLK

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
  os_sys_init (init);                    /* Initialize RTX and start init    */
}

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/

