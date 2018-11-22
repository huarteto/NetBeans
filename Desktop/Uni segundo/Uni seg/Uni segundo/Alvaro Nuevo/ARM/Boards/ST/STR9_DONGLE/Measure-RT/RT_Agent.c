/******************************************************************************/
/* RT_Agent.C: Real-Time Agent V2.0                                            */
/******************************************************************************/
/*                                                                            */                           
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2006 Keil - An ARM Company. All rights reserved.             */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/


#pragma arm

// Real-Time Agent Version
#define RTA_VERSION_MAJ     2
#define RTA_VERSION_MIN     2
#define RTA_VERSION         (RTA_VERSION_MAJ << 4) | RTA_VERSION_MIN

// <<< Use Configuration Wizard in Context Menu >>>

// <e0>Real-Time Agent
// <i>Enable Real-Time Agent to obtain run-time debug information
#ifndef RTA_ENABLED
 #define RTA_ENABLED          1
#endif

#define RTA_HW_LPC21xx        1
#define RTA_HW_LPC23xx        2
#define RTA_HW_STR91x         3
#define RTA_HW_AT91SAM7Sx     4
#define RTA_HW_POLLED         99
#define RTA_HW_CUSTOM         100
// <o0>Hardware Type  <i>Select the target MCU you are using
//   <1=> NXP (Philips) LPC21xx
//   <2=> NXP (Philips) LPC23xx
//   <3=> STMicroelectronics STR91x
//   <4=> Atmel AT91SAM7Sx
//   <99=> Polled Mode (any hardware)
//   <100=> Custom
//   (for custom hardware you need to set macros in the hardware section)
//
#ifndef RTA_HW_TYPE
 #define RTA_HW_TYPE           3
#endif

// <o0>Output Buffer Size (bytes) <i>A slower JTAG and/or greater amount of data requires a larger buffer
//    <256=>256
//    <512=>512
//    <1024=>1024
#ifndef RTA_OUT_SIZE_BYTES
 #define RTA_OUT_SIZE_BYTES  256
#endif
#define RTA_OUT_SIZE         (RTA_OUT_SIZE_BYTES / 4)

// <o>  Invalid Memory Pattern <0x0-0xFFFFFFFF>
// <i>  This pattern is shown at inaccesible locations in the memory window at run-time
#ifndef RTA_NO_MEM_FLAG
 #define RTA_NO_MEM_FLAG     0xCCCCCCCC
#endif

// <e0>Enable Terminal Emulation (for RTA_Term commands)
//   <o1>Input Buffer Size (bytes) <i>A faster JTAG and/or greater amount of data requires a larger buffer
//      <2=>2
//      <4=>4
//      <8=>8
//      <16=>16
//   <o2.0>Echo Characters Automatically<i>Echoes back characters typed in the serial window
//   <o3.0>Block on Write<i>Waits for serial buffer to have available space to send data
//  </e>
#ifndef RTA_TERM_SUPPORT
 #define RTA_TERM_SUPPORT   1
#endif

/* Terminal Emulator input buffer size (must be a power of 2)
     This should be modified based on the amount of
     data being sent, and the speed of the JTAG. A faster
     JTAG and/or greater amount of data would require a 
     larger buffer size. */
#ifndef RTA_TERM_IN_SIZE
 #define RTA_TERM_IN_SIZE    16
#endif

/* Echo characters back to the serial window automatically */
#ifndef RTA_TERM_ECHO
 #define RTA_TERM_ECHO       0
#endif

/* Blocking writes - write  blocks when output buffer full*/
#ifndef RTA_TERM_W_BLOCKING
 #define RTA_TERM_W_BLOCKING 1
#endif

// </e>
// <<< end of configuration section >>>


/*==============================================================================
   DO NOT EDIT BELOW HERE (unless using custom hardware)
==============================================================================*/


/*==============================================================================
   RTA Hardware Specific Section 
==============================================================================*/

#if (RTA_HW_TYPE == RTA_HW_LPC21xx) 

#include <LPC21xx.H>                /* LPC21xx definitions  */

#define RTA_EXPORT_IRQS   1

#define TX_IRQ_INIT()     VICVectAddr13 = (unsigned long)RTA_tx_word; \
                          VICVectCntl13 = 0x20 | 3;                      
#define TX_IRQ_ACK()      VICVectAddr = 0;
#define TX_IRQ_ENABLE()   VICIntEnable = 0x00000008;
#define TX_IRQ_DISABLE()  VICIntEnClr = 0x00000008;
                                            
#define RX_IRQ_INIT()     VICVectAddr14 = (unsigned long)RTA_rx_word; \
                          VICVectCntl14 = 0x20 | 2;               
#define RX_IRQ_ACK()      VICVectAddr = 0; 
#define RX_IRQ_ENABLE()   VICIntEnable = 0x00000004;
#define RX_IRQ_DISABLE()  VICIntEnClr = 0x00000004;

#define DEF_IRQ_INIT()    VICDefVectAddr= (unsigned long)RTA_def_irq;
#define DEF_IRQ_ACK()     VICVectAddr = 0;

#define TX_WORD()         __asm { MCR     p14, 0, tx_word, c1, c0 }
#define RX_WORD()         __asm { MRC     p14, 0, rx_word, c1, c0 }
 
 
#elif (RTA_HW_TYPE == RTA_HW_LPC23xx) 

#include <LPC23xx.H>                /* LPC23xx definitions  */

#define RTA_EXPORT_IRQS   0

#define TX_IRQ_INIT()     VICVectAddr3 = (unsigned long)RTA_tx_word; \
                          VICVectCntl3 = 0x0f;                      
#define TX_IRQ_ACK()      VICVectAddr = 0;
#define TX_IRQ_ENABLE()   VICIntEnable = 0x00000008;
#define TX_IRQ_DISABLE()  VICIntEnClr = 0x00000008;
                                            
#define RX_IRQ_INIT()     VICVectAddr2 = (unsigned long)RTA_rx_word; \
                          VICVectCntl2 = 0x0f;               
#define RX_IRQ_ACK()      VICVectAddr = 0; 
#define RX_IRQ_ENABLE()   VICIntEnable = 0x00000004;
#define RX_IRQ_DISABLE()  VICIntEnClr = 0x00000004;

#define DEF_IRQ_INIT()    
#define DEF_IRQ_ACK()     

#define TX_WORD()         __asm { MCR     p14, 0, tx_word, c1, c0 }
#define RX_WORD()         __asm { MRC     p14, 0, rx_word, c1, c0 }
 
 
#elif (RTA_HW_TYPE == RTA_HW_STR91x)

#include <91x_lib.h> 		    /* STR91x definitions        */ 

#define RTA_EXPORT_IRQS   0

#define TX_IRQ_INIT()     VIC0->VAiR[3] = (unsigned int)RTA_tx_word; \
                          VIC0->INTSR  &= ~(0x01 << (ARMTX_ITLine)); \
                          VIC0->VCiR[3] =    0x20 | (ARMTX_ITLine); 
#define TX_IRQ_ACK()      VIC0->VAR = 0; \
                          VIC1->VAR = 0;
#define TX_IRQ_ENABLE()   VIC0->INTER  |=  (0x01 << (ARMTX_ITLine));
#define TX_IRQ_DISABLE()  VIC0->INTECR |=  (0x01 << (ARMTX_ITLine));
                                                            
#define RX_IRQ_INIT()     VIC0->VAiR[2] = (unsigned int)RTA_rx_word; \
                          VIC0->INTSR  &= ~(0x01 << (ARMRX_ITLine)); \
                          VIC0->VCiR[2] =    0x20 | (ARMRX_ITLine);                         
#define RX_IRQ_ACK()      VIC0->VAR = 0; \
                          VIC1->VAR = 0;
#define RX_IRQ_ENABLE()   VIC0->INTER  |=  (0x01 << (ARMRX_ITLine));
#define RX_IRQ_DISABLE()  VIC0->INTECR |=  (0x01 << (ARMRX_ITLine));

#define DEF_IRQ_INIT()    VIC0->DVAR    = (unsigned long)RTA_def_irq;
#define DEF_IRQ_ACK()     VIC0->VAR = 0; \
                          VIC1->VAR = 0;

#define TX_WORD()         __asm { MCR     p14, 0, tx_word, c1, c0 }
#define RX_WORD()         __asm { MRC     p14, 0, rx_word, c1, c0 }


#elif (RTA_HW_TYPE == RTA_HW_AT91SAM7Sx)

#include <AT91SAM7S64.H>  	    /* AT91SAM7Sx definitions        */ 

#define RTA_EXPORT_IRQS   1      

#define TX_IRQ_INIT()     // In user code
#define TX_IRQ_ACK()      // In user code
#define TX_IRQ_ENABLE()   *AT91C_DBGU_IER = AT91C_US_COMM_TX;   
#define TX_IRQ_DISABLE()  *AT91C_DBGU_IDR = AT91C_US_COMM_TX;
           
#define RX_IRQ_INIT()     // In user code          
#define RX_IRQ_ACK()      // In user code          						   
#define RX_IRQ_ENABLE()   *AT91C_DBGU_IER = AT91C_US_COMM_RX;
#define RX_IRQ_DISABLE()  *AT91C_DBGU_IDR = AT91C_US_COMM_RX;
       
#define DEF_IRQ_INIT()    *AT91C_AIC_SPU = (unsigned long) RTA_def_irq;
#define DEF_IRQ_ACK()     *AT91C_AIC_EOICR = 0;

#define TX_WORD()         __asm { MCR     p14, 0, tx_word, c1, c0 }
#define RX_WORD()         __asm { MRC     p14, 0, rx_word, c1, c0 }


#elif (RTA_HW_TYPE == RTA_HW_POLLED)

#define RTA_EXPORT_IRQS   1      

#define TX_IRQ_INIT()     // Not used
#define TX_IRQ_ACK()      // Not used
#define TX_IRQ_ENABLE()   // Not used
#define TX_IRQ_DISABLE()  // Not used
           
#define RX_IRQ_INIT()     // Not used
#define RX_IRQ_ACK()      // Not used
#define RX_IRQ_ENABLE()   // Not used
#define RX_IRQ_DISABLE()  // Not used
       
#define DEF_IRQ_INIT()    // In user code
#define DEF_IRQ_ACK()     // In user code

#define TX_WORD()         __asm { MCR     p14, 0, tx_word, c1, c0 }
#define RX_WORD()         __asm { MRC     p14, 0, rx_word, c1, c0 }

#define TX_WORD_POSS      __asm { MRC     p14,0,r,c0,c0   } \
                          __asm { AND     r,r,#0x02       } \
                          __asm { EOR     r,r,#0x02       }
#define RX_WORD_POSS      __asm { MRC     p14,0,r,c0,c0   } \
                          __asm { AND     r,r,#0x01       }


#elif (RTA_HW_TYPE == RTA_HW_CUSTOM)  /* Custom hardare */

#define RTA_EXPORT_IRQS   0

#define TX_IRQ_INIT()  
#define TX_IRQ_ACK() 
#define TX_IRQ_ENABLE()  
#define TX_IRQ_DISABLE()                                                               

#define RX_IRQ_INIT()               
#define RX_IRQ_ACK()    
#define RX_IRQ_ENABLE() 
#define RX_IRQ_DISABLE()

#define DEF_IRQ_INIT()
#define DEF_IRQ_ACK()

#define TX_WORD()         __asm { MCR     p14, 0, tx_word, c1, c0 }      
#define RX_WORD()         __asm { MRC     p14, 0, rx_word, c1, c0 }

#endif  /* RTA_HW_TYPE */


/*==============================================================================
   Real-Time Agent Includes
==============================================================================*/

#include "RT_Agent.H"

#if (RTA_TERM_SUPPORT)
  #include <stdio.h>
#endif


/*==============================================================================
   Real-Time Agent Definitions
==============================================================================*/

// Protocol Constants
#define RTA_TX_MAGIC_NO      0xFA
#define RTA_INITIAL_SEQ_NO   0xCE
#define RTA_SYNC_WORD        0xFA00CE00
#define RTA_SYNC_MSG_SIZE    2
#define RTA_REQ_RSP_BIT      0x80
#define RTA_MAX_DATA_LEN     120          
#define RTA_MAX_MSG_LEN      (RTA_MAX_DATA_LEN + 8)  // Message = Header + Data + Checksum = 128 (32 words)

// Buffer masks
#define RTA_OUT_MASK         (RTA_OUT_SIZE-1ul)
#define RTA_TERM_IN_MASK     (RTA_TERM_IN_SIZE-1ul)

// Functional macros
#define RTA_RESET()          {unsigned int iZero = 0; RTA_Msg(RTA_CMD_UNKNOWN, &iZero, RTA_SHORT_MSG); } // Reset RTA communications
#define RTA_SYNC()           RTA_Msg(RTA_CMD_SYNC, (unsigned int *)&sync_msg_data, RTA_SYNC_MSG_SIZE);   // Sync  RTA communications

// RTA Internal (private) command definitions
typedef enum __RTA_INTERNAL_COMMANDS
{
  RTA_CMD_SYNC            = 0x00,

  /* Normal internal command codes */
  RTA_CMD_ERROR           = 0x01,
  RTA_CMD_TERM_DATA       = 0x02,
  RTA_CMD_SET_ADDRESS     = 0x03,
  RTA_CMD_MEM_WRITE_8     = 0x04,
  RTA_CMD_MEM_WRITE_16    = 0x05,
  RTA_CMD_MEM_WRITE_32    = 0x06,
  RTA_CMD_RTX_TASK_SWITCH = 0x07,
  RTA_CMD_RTX_MON_START   = 0x08,
  RTA_CMD_RTX_MON_STOP    = 0x09,
  RTA_CMD_RTX_TASK_CHANGE = 0x0A,
  RTA_CMD_RTX_CLOCK_INFO  = 0x0B,
  
  /* Synchronous response command codes (must have bit 7 set) */
  RTA_CMD_GET_VERSION     = 0x80,
  RTA_CMD_MEM_READ_8      = 0x81,
  RTA_CMD_MEM_READ_16     = 0x82,
  RTA_CMD_MEM_READ_32     = 0x83,
    
  /* Speical Unknown command definition for reset */
  RTA_CMD_UNKNOWN         = 0x0F     
} RTA_INTERNAL_COMMANDS;

// RTA message formats
typedef enum __MSG_FORMAT {
  MSG_FORMAT_SHORT = 0,
  MSG_FORMAT_LONG,
  MSG_FORMAT_TASKSWITCH
} MSG_FORMAT;

// RTA state machine states
typedef enum __RTA_STATES
{
  RTA_STATE_COMMAND = 0,
  RTA_STATE_ADDRESS,
  RTA_STATE_MEM_WRITE_8,
  RTA_STATE_MEM_WRITE_16,
  RTA_STATE_MEM_WRITE_32,
  RTA_STATE_COUNT                  // Always leave at end
} RTA_STATES;

// RTA message header split
typedef struct __msg_header_t {
  unsigned int length     : 8;    // 0x000000XX
  unsigned int seq_no     : 8;    // 0x0000XX00
  unsigned int type       : 8;    // 0x00XX0000
  unsigned int fixed      : 8;    // 0xXX000000
} msg_header_t;

// RTA message header union
typedef union __msg_header_u {
    msg_header_t split;
    unsigned int raw;
} msg_header_u;

// RTA message structure
typedef struct __RTA_msg_t {
  msg_header_u header;
  const unsigned int* data;
  unsigned int checksum;
} RTA_msg_t;

// RTA RX state machine function pointers typedef 
typedef void (*state_fp)(unsigned int rx_word);

// RTA output buffer
typedef struct __RTA_out_t {
  unsigned int data[RTA_OUT_SIZE];
  unsigned int wr_idx;
  unsigned int rd_idx;
} RTA_out_t;

// RTA Terminal Emulator buffer
typedef struct __RTA_term_in_t {
  unsigned char data[RTA_TERM_IN_SIZE];
  unsigned char wr_idx;
  unsigned char rd_idx;
} RTA_term_in_t;

// RTA structure
typedef struct __RTA_t {
  unsigned int state;             // RTA state machine state
  unsigned char tx_seq_no;        // Transmit sequence number
  unsigned char rtx_mon;          // RTX monitoring enabled

  unsigned int rw_addr;           // Read/Write address
  unsigned int w_end_addr;        // Write end address

  RTA_out_t out;                  // Output buffer
} RTA_t;

/*==============================================================================
   Real-Time Agent Exported Variables
==============================================================================*/

// File pointers for the RTA terminal emulation (RTA_printf, etc)
void * fpRTADbgIn = 0;
void * fpRTADbgOut = 0;


#if RTA_ENABLED  
/*==============================================================================
   ENABLED CODE SECTION 
==============================================================================*/                 

// RTA variables
// Real-Time Agent version
const unsigned int __rta_version = RTA_VERSION;	
static volatile RTA_t rta;        // Agent state structure
static int initialised = 0;       // Initialised boolean

#if (RTA_TERM_SUPPORT)
  static volatile RTA_term_in_t term_in; // Terminal Emulator input buffer
#endif

// Function prototypes
static RTA_STATUS send_msg_raw (RTA_msg_t *msg, MSG_FORMAT format);

#if (RTA_EXPORT_IRQS)
void RTA_rx_word_ext (void);
void RTA_tx_word_ext (void);
#else
static __irq void RTA_rx_word (void);
static __irq void RTA_tx_word (void);
#endif
static __irq void RTA_def_irq (void);
static void main_cmd(unsigned int rx_word);
static void main_addr(unsigned int rx_word);
static void main_mem_rd_32(unsigned int rx_word);
static void main_mem_wr_8(unsigned int rx_word);
static void main_mem_wr_16(unsigned int rx_word);
static void main_mem_wr_32(unsigned int rx_word);


// RTA RX state machine function pointers 
static state_fp const state_funcs[RTA_STATE_COUNT] = {
                                            main_cmd,
                                            main_addr,
                                            main_mem_wr_8,
                                            main_mem_wr_16,
                                            main_mem_wr_32
                                            };

static const unsigned int sync_msg_data[RTA_SYNC_MSG_SIZE] = {0x52415365,
                                                              0x77656C6C};


/* Buffer read / write macros */

/* RTA Host out buffer */
#define OUT_BUF_RESET()     (rta.out.rd_idx = rta.out.wr_idx = 0)
#define OUT_BUF_WR(dataOut) (rta.out.data[RTA_OUT_MASK & rta.out.wr_idx++] = (dataOut))
#define OUT_BUF_RD()        (rta.out.data[RTA_OUT_MASK & rta.out.rd_idx++])   
#define OUT_BUF_EMPTY()     (rta.out.rd_idx == rta.out.wr_idx)
#define OUT_BUF_FULL()      (rta.out.rd_idx == rta.out.wr_idx+1)

/* RTA Host terminal in buffer */
#define TERM_IN_BUF_RESET()     (term_in.rd_idx = term_in.wr_idx = 0)
#define TERM_IN_BUF_WR(dataOut) (term_in.data[RTA_TERM_IN_MASK & term_in.wr_idx++] = (dataOut))
#define TERM_IN_BUF_RD()        (term_in.data[RTA_TERM_IN_MASK & term_in.rd_idx++])
#define TERM_IN_BUF_EMPTY()     (term_in.rd_idx == term_in.wr_idx)
#define TERM_IN_BUF_FULL()      (term_in.rd_idx == term_in.wr_idx+1)
#define TERM_IN_BUF_NOT_EMPTY() ((term_in.wr_idx - term_in.rd_idx) & RTA_TERM_IN_MASK)


/*==============================================================================
   RTA External Functions 
==============================================================================*/

/*
 *  Initialise Real-Time Agent
 *    Parameters:     None
 *    Return Value:   RTA_STATUS (0 = OK)
 */
RTA_STATUS RTA_Init (void) {
  if (!initialised) {                  
    DEF_IRQ_INIT();                             // Set up the default (suprious) interrupt 
    TX_IRQ_INIT();                              // Set up the RTA TX interrupt 
    RX_IRQ_INIT();                              // Set up the RTA RX interrupt
                     
    rta.state = RTA_STATE_COMMAND;              // Reset the states and buffer pointers
    rta.tx_seq_no = RTA_INITIAL_SEQ_NO;
    OUT_BUF_RESET();                            // Set buffer empty conditions

#if (RTA_TERM_SUPPORT)
    TERM_IN_BUF_RESET();                        // Reset terminal buffer    
    fpRTADbgIn = fopen("RTADBGIN", "r");        // Open RTA dbg in for reading               
    fpRTADbgOut = fopen("RTADBGOUT", "w");      // Open RTA dbg out for writeing                           
#endif

    RX_IRQ_ENABLE();                            // Enable RTA Rx Interrupt
    initialised = 1;
  }

  return (RTA_OK);
}

/*
 *  Send a message to the host via Real-Time Agent
 *    Parameters:     type:   type code of command/message
 *                    data:   pointer to message buffer to send
 *                    length: length of data message buffer in words,
 *                            or 0 for a short message
 *    Return Value:   RTA_STATUS (0 = OK)
 */
RTA_STATUS RTA_Msg (unsigned int type, unsigned int *data, unsigned int length) {
  RTA_STATUS status = RTA_OK;
  MSG_FORMAT format;
  RTA_msg_t  msg;

  if (initialised) {
  
    // Create header
    msg.header.split.type = type;
  
    // Add data
    if (length) {                               // Long Message    
      msg.header.split.length = length;
      msg.data = data;
      format = MSG_FORMAT_LONG;
    }
    else {                                      // Short Message
      if (data == 0) return (RTA_ERR_PARAM);    // Parameter check
      msg.header.split.length = *data;
      format = MSG_FORMAT_SHORT;                // Indicates short message    
    }
        
    status = send_msg_raw(&msg, format);        // Send message

  }
  else {
    status = RTA_ERR_NOT_INIT;
  }

  return (status);
}

/*
 *  Send RTX Kernel task switch information to the host via Real-Time Agent
 *    Parameters:     tsk:    packed task switch message
 *    Return Value:   RTA_STATUS (0 = OK)
 */
void RTA_Task_Mon (unsigned int tsk) {
  RTA_msg_t  msg;

	if (rta.rtx_mon) {
	  msg.header.raw = tsk & 0x7FFFFFFF;
	  send_msg_raw(&msg, MSG_FORMAT_TASKSWITCH);
	}
}

/*==============================================================================
   RTA Receive Functions
==============================================================================*/

/*  
 *  Handle a read/write address from the host
 *    Parameters:      rx_word:   received address word
 *    Return Value:    None 
 */
static void main_addr(unsigned int rx_word) {
  rta.rw_addr = rx_word;
  rta.state = RTA_STATE_COMMAND;
}

/*
 *  Handle a mem read from the host
 *    Parameters:      length:   length of data to read in words
 *    Return Value:    None
 */
static void main_mem_rd_32(unsigned int length){
  // Return the data that was requested to read
  RTA_Msg(RTA_CMD_MEM_READ_32, 
          (unsigned int *)rta.rw_addr, 
          length);
}

/*
 *  Handle a 8-bit mem write from the host
 *    Parameters:      data:   data to write
 *    Return Value:    None
 */
static void main_mem_wr_8(unsigned int data) {
  unsigned int byte = 0;

  // Write data (8-bit)
  while ((rta.rw_addr < rta.w_end_addr) && (byte < 32)) {
    *((unsigned char *)rta.rw_addr) = (data >> byte) & 0x000000FF;
    rta.rw_addr += sizeof(unsigned char);
    byte+=8;
  }

  // Switch back to command mode when write finished
  if (rta.rw_addr >= rta.w_end_addr) {
    rta.state = RTA_STATE_COMMAND;  
  }
}

/*
 *  Handle a 16-bit mem write from the host
 *    Parameters:      data:   data to write
 *    Return Value:    None
 */
static void main_mem_wr_16(unsigned int data) {
  unsigned int halfword = 0;

  // Write data (16-bit)
  while ((rta.rw_addr < rta.w_end_addr) && (halfword < 32)) {
    *((unsigned short *)rta.rw_addr) = (data >> halfword) & 0x0000FFFF;
    rta.rw_addr += sizeof(unsigned short);
    halfword+=16;
  } 

  // Switch back to command mode when write finished
  if (rta.rw_addr >= rta.w_end_addr) {
    rta.state = RTA_STATE_COMMAND;  
  }
}

/*
 *  Handle a 32-bit mem write from the host
 *    Parameters:      data:   data to write
 *    Return Value:    None
 */
static void main_mem_wr_32(unsigned int data) {
  // Write data (32-bit)
  *((unsigned int *)rta.rw_addr) = data;
  rta.rw_addr += sizeof(unsigned int);

  // Switch back to command mode when write finished
  if (rta.rw_addr >= rta.w_end_addr) {
    rta.state = RTA_STATE_COMMAND;  
  }
}

/*
 *  Handle commands from the host
 *    Parameters:      rx_word:   command word received
 *                     val:   Data
 *    Return Value:    None
 */
static void main_cmd(unsigned int rx_word) {

  /* Could check sequence number and magic number here, but
     do not because of the processing time cost */
  switch ((rx_word >> 16) & 0xFF) {                    // Message type
    case RTA_CMD_SYNC:
      if (rx_word == RTA_SYNC_WORD) {
        RTA_SYNC();
      }
      else {
        RTA_RESET();
      } 
    break;

    case RTA_CMD_GET_VERSION:
      RTA_Msg(RTA_CMD_GET_VERSION, (unsigned int *)&__rta_version, RTA_SHORT_MSG);
    break;

    case RTA_CMD_TERM_DATA:
#if (RTA_TERM_SUPPORT)
      // Pass term data to term in buffer
      TERM_IN_BUF_WR(rx_word & 0x000000FF);
#endif
    break;

    case RTA_CMD_SET_ADDRESS:
      rta.state = RTA_STATE_ADDRESS;
    break;

    case RTA_CMD_MEM_READ_32:                
      main_mem_rd_32(rx_word & 0x000000FF);
    break;  

    /* RTX monitoring messages */
    case RTA_CMD_RTX_MON_START:
      rta.rtx_mon = 1;
    break;

    case RTA_CMD_RTX_MON_STOP:
      rta.rtx_mon = 0;
    break;

    /* Special mode messages */
    case RTA_CMD_MEM_WRITE_8:
      rta.w_end_addr = rta.rw_addr + ((rx_word & 0x000000FF)); 
      rta.state = RTA_STATE_MEM_WRITE_8;                
    break;

    case RTA_CMD_MEM_WRITE_16:
      rta.w_end_addr = rta.rw_addr + ((rx_word & 0x000000FF) << 1); 
      rta.state = RTA_STATE_MEM_WRITE_16;                
    break;

    case RTA_CMD_MEM_WRITE_32:
      rta.w_end_addr = rta.rw_addr + ((rx_word & 0x000000FF) << 2); 
      rta.state = RTA_STATE_MEM_WRITE_32;                
    break;

    default:
      RTA_RESET();
  }
}

/*
 *  RTA Receive word possible
 *    Parameters:      None
 *    Return Value:    0 = Not possible, Non-zero = possible
 */
#if (RTA_HW_TYPE == RTA_HW_POLLED)
static __forceinline int RTA_rx_word_poss (void)
{
  int r;
  RX_WORD_POSS;
  return (r);
}
#endif /* if (RTA_HW_TYPE == RTA_HW_POLLED) */

/*
 *  RTA Receive word (interrupt context)
 *    Parameters:      None
 *    Return Value:    None
 */
#if (RTA_EXPORT_IRQS)
void RTA_rx_word_ext (void)
#else
__irq void RTA_rx_word (void)
#endif
{
  unsigned int rx_word;

  RX_IRQ_ACK();                               // Acknowledge Interrupt

#if (RTA_HW_TYPE == RTA_HW_POLLED)
  if (RTA_rx_word_poss()) {                   // Check RX buffer free
#endif
    RX_WORD();                                // Read word from hardware                                
    state_funcs[rta.state](rx_word);          // Call state machine
#if (RTA_HW_TYPE == RTA_HW_POLLED)
  }
#endif
}

/*==============================================================================
   RTA Transmit Functions
==============================================================================*/

/*
 *  Write data to the RTA output buffer
 *    Parameters:      data:   data word to write
 *    Return Value:    None
 */
static __forceinline void RTA_out_write(unsigned int data) {
  OUT_BUF_WR(data);
}


/*
 *  Read memory for the data out buffer. This must be a
 *  function call as it needs to be written in 
 *  ASM so that register locations are predicatable
 *  for the Data Abort handler as is may cause a data abort
 *    Parameters:      addrData:   address to read
 *    Return Value:    Data read
 */
__asm unsigned int __RTA_rd_adr (unsigned int *addrData) {
        LDR     R0,[R0,#0]          ; Load the data at addrData
        BX      LR                  ; Return
}

__asm unsigned int DAbt_Handler (void) {
        LDR     R0,=RTA_NO_MEM_FLAG ; Move No_Mem_Flag into R0
        SUBS    PC,LR,#4            ; Return past the data abort instruction
}

/*
 *  Send a message to the host
 *    Parameters:      msg:     pointer to the message
 *                     //format:  format of the message (LONG or SHORT)
 *                     Message format is now determined by a NULL data pointer
 *    Return Value:    RTA_STATUS (0 = OK)
 */
static RTA_STATUS send_msg_raw (RTA_msg_t *msg, MSG_FORMAT format) {
  RTA_STATUS retVal = RTA_OK;
  unsigned int type = msg->header.split.type;
  const unsigned int *data = msg->data;             // Local pointer to the data
  unsigned int length = msg->header.split.length;
  unsigned int temp;                           

  TX_IRQ_DISABLE();                                 // Disable RTA Tx Interrupt

  // Special case for sync (and reset) messages
  if (format != MSG_FORMAT_TASKSWITCH) {
    if ((type == RTA_CMD_UNKNOWN) || (type == RTA_CMD_SYNC)) {
      // Have to reset the states and buffer pointers here with TX IRQ disabled so it is safe
      rta.state = RTA_STATE_COMMAND;
      OUT_BUF_RESET();                                // Set buffer empty condition
      rta.tx_seq_no = RTA_INITIAL_SEQ_NO;             // Reset sequence number
    }
  }

  /* Check there is enough buffer for the message, otherwise discard it
     Note: We leave at least RTA_MAX_MSG_LEN in the buffer at all times
     for req/rsp messages from the host */
  temp = (format == MSG_FORMAT_LONG) ? length : 1;  // Message length
  if ((!(type & RTA_REQ_RSP_BIT)) || (format == MSG_FORMAT_TASKSWITCH)) {
    temp += (RTA_MAX_MSG_LEN/4);                    // Buffer reserved for req/rsp messages
    if (type == RTA_CMD_ERROR) {                    // Buffer reserved for error messages
      temp--;
    } 
  }

  /* If there is no buffer free, send a buffer overrun message to host and
     discard the original message */
  if ( ((rta.out.rd_idx - rta.out.wr_idx - 1) & RTA_OUT_MASK) <   // Buffer free 
       temp) {                                              
    retVal = RTA_ERR_BUF_OVERRUN;
    /* Don't send errors for term data in blocking mode */
    #if RTA_TERM_W_BLOCKING                         
      if (type != RTA_CMD_TERM_DATA) {
    #endif
      // Configure the buffer overrun message         
      msg->header.split.type = RTA_CMD_ERROR;     
      length = msg->header.split.length = RTA_ERR_BUF_OVERRUN;   
    #if RTA_TERM_W_BLOCKING
      }
      else {
        TX_IRQ_ENABLE();
        return (retVal);
      }
    #endif
  } 

  // First put the sequence number into the header
  if (format != MSG_FORMAT_TASKSWITCH) {
    msg->header.split.fixed = RTA_TX_MAGIC_NO;
    msg->header.split.seq_no = (unsigned int)rta.tx_seq_no++;
  }

  // Now write header (or short message) to the buffer
  RTA_out_write(msg->header.raw);

  // Write the long message
  if (format == MSG_FORMAT_LONG) {
    for (; length>0; length--) {      
      /* type reused as temp data store */
      type = __RTA_rd_adr((unsigned int *)data++);
      RTA_out_write(type);
    }
  }

  TX_IRQ_ENABLE();                              // Enable RTA Tx Interrupt

  return (retVal);
}


/*
 *  RTA Transmit word possible
 *    Parameters:      None
 *    Return Value:    0 = Not possible, Non-zero = possible
 */
#if (RTA_HW_TYPE == RTA_HW_POLLED)
static __forceinline int RTA_tx_word_poss (void)
{
  int r;
  TX_WORD_POSS;
  return (r);
}
#endif /* if (RTA_HW_TYPE == RTA_HW_POLLED) */

/*
 *  RTA Transmit word (interrupt context)
 *    Parameters:      None
 *    Return Value:    None
 */
#if (RTA_EXPORT_IRQS)
void RTA_tx_word_ext (void)
#else
__irq void RTA_tx_word (void)
#endif
{
  unsigned int tx_word;

  TX_IRQ_ACK();                                 // Acknowledge Interrupt

  if (OUT_BUF_EMPTY()) {                        // Check for buffer empty
    TX_IRQ_DISABLE();                           // Disable RTA Tx Interrupt                       
  }
  else {                                        // Write data if buffer not empty
#if (RTA_HW_TYPE == RTA_HW_POLLED)
    if (RTA_tx_word_poss()) {                   // Check TX buffer Free
#endif      
      tx_word = OUT_BUF_RD();                   // Write word to the RTA    
      TX_WORD();                                // Write word to hardware                                             
#if (RTA_HW_TYPE == RTA_HW_POLLED)
    }                   
#endif      
  }
}

/*
 *  RTA Default Interrupt (only used when interrupts are used)
 *    Parameters:      None
 *    Return Value:    None
 */
#pragma push
#pragma diag_remark 177                 // Temporarily disable unused function warning
static __irq void RTA_def_irq (void) {
  DEF_IRQ_ACK();
}
#pragma pop  

/*==============================================================================
   RTA Terminal Emulator Functions 
==============================================================================*/

#if (RTA_TERM_SUPPORT)

/*     
 *  Check is there is a character available in the terminal emulator buffer
 *    Parameters:      None
 *    Return Value:    Number of characters in buffer
 */
unsigned int RTA_TermIsKey (void) {
  return (TERM_IN_BUF_NOT_EMPTY());
}

/*
 *  Get a character from the terminal. This function blocks until a
 *  character is available
 *    Parameters:      None
 *    Return Value:    Character read from terminal
 */
unsigned int RTA_TermGetChar (void) {
  int ch;

  while (TERM_IN_BUF_EMPTY());              // Block waiting for character

  ch = TERM_IN_BUF_RD();                    // Read char from buffer

#if (RTA_TERM_ECHO)
  RTA_TermPutChar(ch);                      // Echo the character back to the serial window
#endif
   
  return (ch);
}

/*
 *  Write a character to the terminal
 *    Parameters:      ch:    Character to write to the terminal
 *    Return Value:    Character written to the terminal
 */
unsigned int RTA_TermPutChar (unsigned int ch) {

#if (RTA_TERM_W_BLOCKING)
  RTA_STATUS status;

  do {
    status = RTA_Msg(RTA_CMD_TERM_DATA, &ch, RTA_SHORT_MSG);
  } while (status == RTA_ERR_BUF_OVERRUN);
#else
  RTA_Msg(RTA_CMD_TERM_DATA, &ch, RTA_SHORT_MSG);
#endif

  return (ch);
}

#else  /* if RTA_TERM_SUPPORT */

unsigned int RTA_TermIsKey (void) { 
  return (0); 
}

unsigned int RTA_TermGetChar (void) {
  return (0);
}

unsigned int RTA_TermPutChar (unsigned int ch) {
  return (0);
}

#endif /* else RTA_TERM_SUPPORT */


#else  /* if RTA_ENABLED */
/*==============================================================================
   DISABLED (STUBBED) CODE SECTION 
==============================================================================*/                 


/* The following pragmas and code force the comiler to not optimize away the 
   __rta_version symbol */
#pragma push
#pragma O0                              // Temporarily no optimisation
#pragma diag_remark 177                 // Temporarily disable unused function warning
/* A version of 0 inidcates to the debugger that the RTA is not in the target */
const unsigned int __rta_version = 0;	

RTA_STATUS RTA_Init (void) {  
  volatile unsigned int *i = (volatile unsigned int *)&__rta_version;
  return (RTA_ERR_DISABLED);
}
#pragma pop

RTA_STATUS RTA_Msg (unsigned int type, unsigned int *data, unsigned int length) {
  return (RTA_ERR_DISABLED);
}

unsigned int RTA_TermIsKey (void) { 
  return(0); 
}

unsigned int RTA_TermGetChar (void) {
  return (0);
}

unsigned int RTA_TermPutChar (unsigned int ch) {
  return (0);
}

__asm unsigned int DAbt_Handler (void) {
        B       DAbt_Handler  ; Loop forever
}

#endif /* else RTA_ENABLED */
