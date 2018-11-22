/*----------------------------------------------------------------------------
 *      RL-ARM - RTX
 *----------------------------------------------------------------------------
 *      Name:    RTX_CONFIG.C
 *      Purpose: Configuration of RTX Kernel for Atmel AT91SAM9263
 *      Rev.:    V4.05
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2009 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include <RTL.h>
#include <AT91SAM9263.H>                 /* AT91SAM9263 definitions          */
#include <RT_Agent.h>                    /* Real-Time Agent definitions      */

/*----------------------------------------------------------------------------
 *      RTX User configuration part BEGIN
 *---------------------------------------------------------------------------*/

//-------- <<< Use Configuration Wizard in Context Menu >>> -----------------
//
// <h>Task Configuration
// =====================
//
//   <o>Number of concurrent running tasks <0-250>
//   <i> Define max. number of tasks that will run at the same time.
//   <i> Default: 6
#ifndef OS_TASKCNT
 #define OS_TASKCNT     6
#endif

//   <o>Number of tasks with user-provided stack <0-250>
//   <i> Define the number of tasks that will use a bigger stack.
//   <i> The memory space for the stack is provided by the user.
//   <i> Default: 0
#ifndef OS_PRIVCNT
 #define OS_PRIVCNT     0
#endif

//   <o>Task stack size [bytes] <20-4096:8><#/4>
//   <i> Set the stack size for tasks which is assigned by the system.
//   <i> Default: 200
#ifndef OS_STKSIZE
 #define OS_STKSIZE     50
#endif

// <q>Check for the stack overflow
// ===============================
// <i> Include the stack checking code for a stack overflow.
// <i> Note that additional code reduces the RTX performance.
#ifndef OS_STKCHECK
 #define OS_STKCHECK    1
#endif

// </h>
// <h>Tick Timer Configuration
// =============================
//   <o>Hardware timer <0=> Timer 0 <1=> Timer 1 <2=> Timer 2 <3=> PIT
//   <i> Define the on-chip timer used as a time-base for RTX.
//   <i> Default: PIT
#ifndef OS_TIMER
 #define OS_TIMER       3
#endif

//   <q>Common IRQ System Handler for PIT timer
//   <i> Include a code for Common System Interrupt Handler
//   <i> when a PIT timer is used
#ifndef OS_SYSIRQ
 #define OS_SYSIRQ      1
#endif

//   <o>Timer clock value [Hz] <1-1000000000>
//   <i> Set the timer clock value for selected timer.
//   <i> Default PIT: 6000000  (6 MHz at 96MHz MCLK and prescaler by 16)
//   <I> Default TCx: 3000000  (3 MHz at 96MHz MCLK and prescaler by 32)
#ifndef OS_CLOCK
 #define OS_CLOCK       6000000
#endif

//   <o>Timer tick value [us] <1-1000000>
//   <i> Set the timer tick value for selected timer.
//   <i> Default: 10000  (10ms)
#ifndef OS_TICK
 #define OS_TICK        10000
#endif

// </h>

// <h>System Configuration
// =======================
// <e>Round-Robin Task switching
// =============================
// <i> Enable Round-Robin Task switching.
#ifndef OS_ROBIN
 #define OS_ROBIN       1
#endif

//   <o>Round-Robin Timeout [ticks] <1-1000>
//   <i> Define how long a task will execute before a task switch.
//   <i> Default: 5
#ifndef OS_ROBINTOUT
 #define OS_ROBINTOUT   5
#endif

// </e>

//   <o>Number of user timers <0-250>
//   <i> Define max. number of user timers that will run at the same time.
//   <i> Default: 0  (User timers disabled)
#ifndef OS_TIMERCNT
 #define OS_TIMERCNT    0
#endif

//   <o>ISR FIFO Queue size<4=>   4 entries  <8=>   8 entries
//                         <12=> 12 entries  <16=> 16 entries
//                         <24=> 24 entries  <32=> 32 entries
//                         <48=> 48 entries  <64=> 64 entries
//                         <96=> 96 entries
//   <i> ISR functions store requests to this buffer,
//   <i> when they are called from the IRQ handler.
//   <i> Default: 16 entries
#ifndef OS_FIFOSZ
 #define OS_FIFOSZ      16
#endif

// </h>

// <e0>Real-Time Agent
// <i>Enable Real-Time Agent to obtain run-time debug information.
#ifndef __RTA_ENABLED
 #define __RTA_ENABLED         1
#endif

// <o0>Output Buffer Size (bytes) <i>If you are experiencing buffer overruns, increase the buffer size.
// <i>Default: 256
//    <256=>256
//    <512=>512
//    <1024=>1024
//    <2048=>2048
#ifndef __RTA_OUT_SIZE_BYTES
 #define __RTA_OUT_SIZE_BYTES  512
#endif
#define __RTA_OUT_SIZE         (__RTA_OUT_SIZE_BYTES / 4)

// <o>  Invalid Memory Pattern <0x0-0xFFFFFFFF>
// <i>  This pattern is shown at inaccessible locations in the memory window at run-time.
// <i>  Default: 0xCCCCCCCC
#ifndef __RTA_NO_MEM_FLAG
 #define __RTA_NO_MEM_FLAG     0xCCCCCCCC
#endif

// <e0>Enable Real-Time Agent Terminal
//   <o1.0>Echo Characters Automatically<i>Echoes back characters typed in the serial window
//          <i>Default: Off
//   <o2.0>Block on Write<i>Waits for serial buffer to have available space to send data.
//          <i>This option must be disabled to run without the debugger.
//          <i>Default: On
//  </e>
#ifndef __RTA_TERM_SUPPORT
 #define __RTA_TERM_SUPPORT    1
#endif

/* Echo characters back to the serial window automatically */
#ifndef __RTA_TERM_ECHO
 #define __RTA_TERM_ECHO       0
#endif

/* Blocking writes - write  blocks when output buffer full*/
#ifndef __RTA_TERM_W_BLOCKING
 #define __RTA_TERM_W_BLOCKING 1
#endif

// <o0.0>Task switch monitoring (RTX Kernel only)
// =============================
// <i> Enable Real-Time Agent task switch monitoring.
#ifndef __RTA_RTX_TASK_SWITCHING
 #define __RTA_RTX_TASK_SWITCHING     1
#endif

// </e>

// <e0>I/O Retargeting
// <i>Enable I/O retargeting
#ifndef __IO_RETARGET_ENABLED
 #define __IO_RETARGET_ENABLED 1
#endif

/* Possible stream sources/destinations */
#define __IO_RETARGET_STREAM_NONE           0
#define __IO_RETARGET_STREAM_RTA            1
#define __IO_RETARGET_STREAM_COM_0          2
#define __IO_RETARGET_STREAM_COM_1          3

// <o0>STDIO Routing (for printf(), scanf(), etc) <i>Select the route for standard input/output, and standard error
//   <0=> None
//   <1=> Real-Time Agent
//   <3=> COM Port 1
#ifndef __IO_RETARGET_STDIO_ROUTE
 #define __IO_RETARGET_STDIO_ROUTE          1
#endif


// <o0.0>External file operations (eg RL Flash File System)
// <i>Enable if __fopen(), __fclose(), __write(), __read(), __setfpos(), __flushbuf() and __get_flen() are defined
#ifndef __IO_RETARGET_FOPEN_ETC_ENABLED
 #define __IO_RETARGET_FOPEN_ETC_ENABLED    0
#endif

// </e>

//------------- <<< end of configuration section >>> -----------------------

/* Terminal Emulator input buffer size (must be a power of 2)
     This should be modified based on the amount of
     data being sent, and the speed of the JTAG. A faster
     JTAG and/or greater amount of data would require a 
     larger buffer size. */
#ifndef __RTA_TERM_IN_SIZE
 #define __RTA_TERM_IN_SIZE   16
#endif

/* Override the RTA_Config.c hardware type */
#define RTA_HW_LPC21xx        1
#define RTA_HW_LPC23xx        2
#define RTA_HW_STR91x         3
#define RTA_HW_AT91SAM7Sx     4
#define RTA_HW_AT91SAM7Xx     5
#define RTA_HW_LPC22xx        6
#define RTA_HW_AT91SAM9261    7
#define RTA_HW_AT91SAM9263    8
#define RTA_HW_POLLED         99
#define RTA_HW_CUSTOM         100

#ifndef __RTA_HW_TYPE
 #define __RTA_HW_TYPE        RTA_HW_AT91SAM9263
#endif

/* RTA is configured in file RTX_Config.c */
#define __RTA_RTX_CONFIG      1

// RT Agent configuration check
#if (__RTA_ENABLED == 1)
 #if !((OS_TIMER == 3 && OS_SYSIRQ == 1))
  #error Real-Time Agent Support requires PIT timer and Common IRQ System Handler for PIT timer to be enabled.
 #endif
#endif

// Standard library system mutexes
// ===============================
//  Define max. number system mutexes that are used to protect 
//  the arm standard runtime library. For microlib they are not used.
#ifndef OS_MUTEXCNT
 #define OS_MUTEXCNT    8
#endif

/*----------------------------------------------------------------------------
 *      RTX User configuration part END
 *---------------------------------------------------------------------------*/

#define AIC             AT91C_BASE_AIC                  /* Abbreviations    */
#define PITC            AT91C_BASE_PITC
#define PMC             AT91C_BASE_PMC

#if   (OS_TIMER == 0)                                   /* Timer/Counter 0  */
 #define ID_TC          AT91C_ID_TC012
 #define TCx            AT91C_BASE_TC0
#elif (OS_TIMER == 1)                                   /* Timer/Counter 1  */
 #define ID_TC          AT91C_ID_TC012
 #define TCx            AT91C_BASE_TC1
#elif (OS_TIMER == 2)                                   /* Timer/Counter 2  */
 #define ID_TC          AT91C_ID_TC012
 #define TCx            AT91C_BASE_TC2
#elif (OS_TIMER == 3)                                   /* PIT              */
 #define ID_TC          AT91C_ID_SYS
#else
 #error OS_TIMER invalid
#endif

#define OS_TIM_         (1 << ID_TC)                    /*  Interrupt Mask  */
#define OS_TRV          ((((1ULL*OS_CLOCK*OS_TICK))/1000000)-1)

#if (OS_TIMER < 3) 
 #define OS_TVAL        (TCx->TC_CV & 0x0000FFFF)       /*  Timer Value     */
 #define OS_TOVF        ((TCx->TC_SR >> 4) & 1)         /*  Reload Flag     */
 #define OS_TFIRQ()     AIC->AIC_ISCR  = OS_TIM_;       /*  Force Interrupt */
 #define OS_TIACK()     AIC->AIC_ICCR  = OS_TIM_;       /*  Interrupt Ack   */ \
                        AIC->AIC_EOICR = TCx->TC_SR;
 #define OS_TINIT()     PMC->PMC_PCER  = OS_TIM_;       /*  Initialization  */ \
                        TCx->TC_CCR    = AT91C_TC_CLKEN | AT91C_TC_SWTRG;      \
                        TCx->TC_CMR    = 2 | AT91C_TC_CPCTRG;                  \
                        TCx->TC_RC     = OS_TRV;                               \
                        TCx->TC_IER    = AT91C_TC_CPCS;                        \
                        AIC->AIC_SPU   = (U32)os_def_interrupt;                \
                        AIC->AIC_SVR[ID_TC] = (U32)os_clock_interrupt;         \
                        AIC->AIC_SMR[ID_TC] =                                  \
                          AT91C_AIC_SRCTYPE_INT_POSITIVE_EDGE | 0;
#else
 #define OS_TVAL        (PITC->PITC_PIIR & 0x000FFFFF)  /*  Timer Value     */
 #define OS_TOVF        (PITC->PITC_PISR & 1)           /*  Overflow Flag   */
 #define OS_TFIRQ()     SET_IRQFLAG;                    /*  Force Interrupt */ \
                        AIC->AIC_ISCR  = OS_TIM_;
 #define OS_TIACK()     CLR_IRQFLAG;                    /*  Interrupt Ack   */ \
                        AIC->AIC_ICCR  = OS_TIM_;                              \
                        AIC->AIC_EOICR = PITC->PITC_PIVR;
 #define OS_TINIT()     PITC->PITC_PIMR = OS_TRV |      /*  Initialization  */ \
                          AT91C_PITC_PITIEN | AT91C_PITC_PITEN;                \
                        AIC->AIC_SPU = (U32)os_def_interrupt;                  \
                        AIC->AIC_SVR[ID_TC] = (U32)sys_ctrl_interrupt;         \
                        AIC->AIC_SMR[ID_TC] =                                  \
                          AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL | 0;
#endif

#define OS_IACK()       AIC->AIC_EOICR = 0;             /* Interrupt Ack    */

#define OS_LOCK()       AIC->AIC_IDCR  = OS_TIM_;       /* Lock             */
#define OS_UNLOCK()     AIC->AIC_IECR  = OS_TIM_;       /* Unlock           */

/* WARNING: Using IDLE mode might cause you troubles while debugging. */
#define _idle_()        SYS->PMC_SCDR = 1;

#if (OS_TIMER == 3 && OS_SYSIRQ == 1)
 BIT force_irq;
 #define SET_IRQFLAG    force_irq = __TRUE
 #define CLR_IRQFLAG    force_irq = __FALSE
#else
 #define SET_IRQFLAG
 #define CLR_IRQFLAG
#endif

/*----------------------------------------------------------------------------
 *      Global Functions
 *---------------------------------------------------------------------------*/

extern void os_clock_interrupt (void);

#pragma diag_suppress 61,68

#if (OS_TIMER == 3 && OS_SYSIRQ == 1)
/*--------------------------- sys_irq_handler -------------------------------*/

__irq void irq_sys_handler (void) {
  /* Common System Interrupt Handler for: DBGU, RSTC, RTT, WDT and PMC  */
  /* system peripheral interrupts.                                      */

#if (__RTA_ENABLED == 1)
  /* Real-Time Agent interrupt handling */

  if (*AT91C_DBGU_CSR & *AT91C_DBGU_IMR & AT91C_US_COMM_RX) {
    /* Process COMM_RX interrupt. */
    RTA_rx_word_ext();
  }

  if (*AT91C_DBGU_CSR & *AT91C_DBGU_IMR & AT91C_US_COMM_TX) {
    /* Process COMM_TX interrupt. */ 
    RTA_tx_word_ext();
  }
#endif
  /* Add your interrupt handling here */ 

  /* Acknowledge Interrupt */
  OS_IACK();      
}


/*--------------------------- sys_ctrl_interrupt ----------------------------*/

__asm void sys_ctrl_interrupt (void) {
   /* Common System Interrupt Handler entry. */
        PRESERVE8
        ARM

        STMDB   SP!,{R0}                        ; Save Work Register
        LDR     R0,=__cpp((U32)AT91C_PITC_PISR) ; PIT Status Register
        LDR     R0,[R0]                         ; Read PIT ISR

        TST     R0,#__cpp(AT91C_PITC_PITS)      ; Check for PIT interrupt
        LDMIANE SP!,{R0}                        ; Restore Work Register
        LDRNE   PC,=__cpp(os_clock_interrupt)   ; Jump to RTOS Clock IRQ

        LDR     R0,=__cpp(&force_irq)           ; Check for forced interrupt
        LDRB    R0,[R0]                         ; Read os_psh_flag

        CMP     R0,#__cpp(__TRUE)               ; Check if __TRUE
        LDMIA   SP!,{R0}                        ; Restore Work Register
        LDREQ   PC,=__cpp(os_clock_interrupt)   ; Jump to RTOS Clock IRQ

        LDR     PC,=__cpp(irq_sys_handler)      ; Jump to SYS IRQ Handler
}
#else
 #define sys_ctrl_interrupt   os_clock_interrupt
#endif


/*--------------------------- os_idle_demon ---------------------------------*/

__task void os_idle_demon (void) {
  /* The idle demon is a system task, running when no other task is ready */
  /* to run. The 'os_xxx' function calls are not allowed from this task.  */

  for (;;) {
  /* HERE: include optional user code to be executed when no task runs.*/
  }
}


/*--------------------------- os_tmr_call -----------------------------------*/

void os_tmr_call (U16 info) {
  /* This function is called when the user timer has expired. Parameter  */
  /* 'info' holds the value, defined when the timer was created.         */

  /* HERE: include optional user code to be executed on timeout. */
}


/*--------------------------- os_error --------------------------------------*/

void os_error (U32 err_code) {
  /* This function is called when a runtime error is detected. Parameter */
  /* 'err_code' holds the runtime error code (defined in RTL.H).         */

  /* HERE: include optional code to be executed on runtime error. */
  for (;;);
}


/*----------------------------------------------------------------------------
 *      RTX Configuration Functions
 *---------------------------------------------------------------------------*/

static void os_def_interrupt (void) __irq  {
  /* Default Interrupt Function: may be called when timer ISR is disabled */
  OS_IACK();
}


#include <RTX_lib.c>

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/
