/*----------------------------------------------------------------------------
 * Name:    CAN_tmpm364.c
 * Purpose: CAN interface functions for Toshiba TMPM364
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

#include <CAN.h>                      /* CAN function prototypes             */
#include <TMPM364.h>                  /* TMPM364 definitions                 */

#define MAX_OBJ            32         /* Number of hardware available objects*/

#define sCANMB            (TSB_CANMB_TypeDef *)

/* Values of bit time register for different baudrates
   SP = Sample point     = ((TSEG1+1)/(1+TSEG1+1+TSEG2+1)) * 100%
                                       SJW | SAM | TSEG1 | TSEG2 | TQs |  SP */
const int CAN_BIT_TIME[] = {/*        -----|-----|-------|-------|-----|-----*/
                           0x00000000,/*             unsupported             */
                           0x00000000,/*             unsupported             */
                           0x00000000,/*             unsupported             */
                           0x00000000,/*             unsupported             */
                           0x00000000,/*             unsupported             */
                           0x00000000,/*             unsupported             */
                           0x00000000,/*             unsupported             */
                           0x00000000,/*             unsupported             */
                           0x00000114,/* 1 |  0  |   4   |   1   |  8  | 75% */
                           0x00000224,/* 2 |  0  |   4   |   2   |  9  | 67% */
                           0x00000225,/* 2 |  0  |   5   |   2   | 10  | 70% */
                           0x00000226,/* 2 |  0  |   6   |   2   | 11  | 72% */
                           0x00000336,/* 3 |  0  |   6   |   3   | 12  | 67% */
                           0x00000337,/* 3 |  0  |   7   |   3   | 13  | 69% */
                           0x00000338,/* 3 |  0  |   8   |   3   | 14  | 71% */
                           0x00000348,/* 3 |  0  |   8   |   4   | 15  | 67% */
                           0x00000349,/* 3 |  0  |   9   |   4   | 16  | 69% */
                           0x0000034A,/* 3 |  0  |  10   |   4   | 17  | 71% */
                           0x0000035A,/* 3 |  0  |  10   |   5   | 18  | 67% */
                           0x0000035B,/* 3 |  0  |  11   |   5   | 19  | 68% */
                           0x0000035C,/* 3 |  0  |  12   |   5   | 20  | 70% */
                           0x0000035D,/* 3 |  0  |  13   |   5   | 21  | 71% */
                           0x0000036D,/* 3 |  0  |  13   |   6   | 22  | 68% */
                           0x0000036E,/* 3 |  0  |  14   |   6   | 23  | 70% */
                           0x0000037E,/* 3 |  0  |  14   |   7   | 24  | 67% */
                           0x0000037F,/* 3 |  0  |  15   |   7   | 25  | 68% */
                           };

/* Mailbox object addresses                                                  */
static TSB_CANMB_TypeDef *table_mailbox_addr[MAX_OBJ] = { 
  sCANMB TSB_CANMB0_BASE,  sCANMB TSB_CANMB1_BASE,  sCANMB TSB_CANMB2_BASE,  sCANMB TSB_CANMB3_BASE,  
  sCANMB TSB_CANMB4_BASE,  sCANMB TSB_CANMB5_BASE,  sCANMB TSB_CANMB6_BASE,  sCANMB TSB_CANMB7_BASE,  
  sCANMB TSB_CANMB8_BASE,  sCANMB TSB_CANMB9_BASE,  sCANMB TSB_CANMB10_BASE, sCANMB TSB_CANMB11_BASE, 
  sCANMB TSB_CANMB12_BASE, sCANMB TSB_CANMB13_BASE, sCANMB TSB_CANMB14_BASE, sCANMB TSB_CANMB15_BASE, 
  sCANMB TSB_CANMB16_BASE, sCANMB TSB_CANMB17_BASE, sCANMB TSB_CANMB18_BASE, sCANMB TSB_CANMB19_BASE, 
  sCANMB TSB_CANMB20_BASE, sCANMB TSB_CANMB21_BASE, sCANMB TSB_CANMB22_BASE, sCANMB TSB_CANMB23_BASE, 
  sCANMB TSB_CANMB24_BASE, sCANMB TSB_CANMB25_BASE, sCANMB TSB_CANMB26_BASE, sCANMB TSB_CANMB27_BASE, 
  sCANMB TSB_CANMB28_BASE, sCANMB TSB_CANMB29_BASE, sCANMB TSB_CANMB30_BASE, sCANMB TSB_CANMB31_BASE 
};

/************************* CAN Hardware Configuration ************************/

// *** <<< Use Configuration Wizard in Context Menu >>> ***

// <o> CAN Peripheral Clock Frequency <1-64000000>
//     <i> Peripheral clock frequency with which CAN controller is clocked
//     <i> It is equal to Fsys / 4
//     <i> Default: 12000000
#define CAN_PERI_FREQ         12000000

// *** <<< End of Configuration section             >>> ***


/*----------------------------------------------------------------------------
 *      CAN Driver Functions
 *----------------------------------------------------------------------------
 *  Functions implemented in this module:
 *    static int can_set_baudrate (int ctrl, int baudrate)
 *           int can_testmode     (int ctrl, int testmode)
 *           int can_setup        (int ctrl)
 *           int can_init         (int ctrl, int baudrate)
 *           int can_start        (int ctrl)
 *           int can_receive      (int ctrl, int ch, CAN_msg *msg)
 *           int can_send         (int ctrl,         CAN_msg *msg)
 *           int can_rx_object    (int ctrl, int ch, int id, int object_para)
 *           int can_tx_object    (int ctrl, int ch,         int object_para)
 *---------------------------------------------------------------------------*/


/************************* Auxiliary Functions *******************************/

/*--------------------------- can_set_baudrate ------------------------------*/

static int can_set_baudrate (int ctrl, int baudrate)  {
  int i, val1, val2, min_index;

  /* Find the total time quanta that makes best fit for baudrate and 
     CAN clock frequency                                                     */
  min_index = 8;
  val1 = (CAN_PERI_FREQ % (baudrate*8))/ 8;
  for (i = 9; i < 26; i++) {
    val2 = (CAN_PERI_FREQ % (baudrate*i))/ i;
    if (val1 > val2) {
      val1 = val2;
      min_index = i;
    }
  }

  /* BAUDRATE PRESCALER - with rounding                                      */
  val1 = min_index*baudrate;
  val2 = ((CAN_PERI_FREQ+(val1>>1))/val1) - 1;
  TSB_CAN->BCR1 = val2 & 0xFF;
  TSB_CAN->BCR2 = CAN_BIT_TIME[min_index];

  if (val2 > 0xFF) 
    return 1;

  return 0;
}


/*************************** Module Functions ********************************/

/*--------------------------- can_testmode ----------------------------------*/

int can_testmode (int ctrl, int testmode)  {

  if (!(TSB_CAN->GSR & 0x80)) {           /* If CCE inactive                 */
    TSB_CAN->MCR |= 0x800;                /* Activate SUR (suspend mode req) */
    while (!(TSB_CAN->GSR & 0x100));      /* While !SUA (wait suspend mode)  */
  }

  if (testmode)
    TSB_CAN->MCR |=  0x600;               /* Set   INTLB and TSTLB bits      */
  else
    TSB_CAN->MCR &= ~0x600;               /* Clear INTLB and TSTLB bits      */

  if (!(TSB_CAN->GSR & 0x80)) {           /* If CCE inactive                 */
    TSB_CAN->MCR |= 0x800;                /* Deactivate SUR (normal mode req)*/
    while (TSB_CAN->GSR & 0x100);         /* While SUA  (wait normal mode)   */
  }

  return 0;
}

/*--------------------------- can_setup -------------------------------------*/

int can_setup (int ctrl)  {

  /* Enable CAN pins                                                         */
  TSB_PE->CR  |=  0x10;                   /* Set TX pin as output            */
  TSB_PE->FR3 |=  0x30;                   /* Enable TX and RX pin functions  */
  TSB_PE->OD  &= ~0x10;                   /* Set TX pin as CMOS output       */
  TSB_PE->PUP &= ~0x30;                   /* Disable pull-ups for CAN pins   */
  TSB_PE->IE  |=  0x20;                   /* Set RX pin as input             */

  return 0;
}

/*--------------------------- can_init --------------------------------------*/

int can_init (int ctrl, int baudrate)  {

  if (can_setup (ctrl) != 0)                          /* Setup pins          */
    return 1;

  if (can_set_baudrate(ctrl, baudrate) != 0)          /* Set baudrate        */
    return 1;

  TSB_CAN->MC   = 0;                      /* Disable all mailboxes           */

  TSB_CAN->MCR |= (1 << 3);               /* Set MTOS (tx depends on ID prio */

  return 0;
}

/*--------------------------- can_start -------------------------------------*/

int can_start (int ctrl)  {

  TSB_CAN->MCR &= ~0x80;                  /* Clear CCR (req normal operation */
  while (TSB_CAN->GSR & 0x80);            /* While CCE (chg config enabled)  */

  return 0;
}

/*--------------------------- can_receive -----------------------------------*/

int can_receive (int ctrl, int ch, CAN_msg *msg)  {
  TSB_CANMB_TypeDef *MailBox = table_mailbox_addr[ch-1];    /* Mailbox adres */
  unsigned int   reg_ID, reg_TSVMCF;
  unsigned int  *msg_data;

  if (!(TSB_CAN->RMP & (1 << (ch-1))))    /* If message not received         */
    return 1;

  msg->ch       =  ch;                    /* Channel information             */

  /* Read message identifier                                                 */
  reg_ID        =  MailBox->ID;
  msg->format   = (reg_ID & (1UL << 31)) == (1UL << 31);

  /* Read time stamp value / message control field                           */
  reg_TSVMCF    =  MailBox->TSVMCF;
  msg->type     = (reg_TSVMCF & (1   <<  4)) == (1   <<  4);
  msg->len      = (unsigned char)(reg_TSVMCF & 0x0F);

  msg->id       =  reg_ID & 0x1FFFFFFF;   /* Extended ID                     */
  if (msg->format == STANDARD_FORMAT)     /* Standard message received       */
    msg->id   >>= 18;                     /* Standard ID                     */

  /* Read the data if received message was DATA FRAME                        */
  if (msg->type == DATA_FRAME) {     
    msg_data    = (unsigned int *) &msg->data[0];
    *msg_data++ =  MailBox->DL;           /* Read  first 4 data bytes        */
    *msg_data   =  MailBox->DH;           /* Read second 4 data bytes        */
  }
  TSB_CAN->RMP = (1 << (ch-1));           /* Clear receive flag              */

  return 0;
}

/*--------------------------- can_send --------------------------------------*/

int can_send (int ctrl, CAN_msg *msg)  {
  TSB_CANMB_TypeDef *MailBox  = table_mailbox_addr[msg->ch-1];  /* Mbx adres */
  unsigned int   mbx_mask = (1 << (msg->ch-1));
  unsigned int  *msg_data;

  /* Mailbox 31 can only be used for receive                                 */
  if (msg->ch >= 32) 
    return 1;

  TSB_CAN->MC     &= ~mbx_mask;           /* Disable mailbox                 */
  TSB_CAN->MD     &= ~mbx_mask;           /* Set mailbox direction (tx)      */

  /* Setup the identifier information                                        */
  if (msg->format == STANDARD_FORMAT)     /* Standard ID                     */
    MailBox->ID     = (msg->id & 0x000007FF) << 18;
  else                                    /* Extended ID                     */
    MailBox->ID     = (msg->id & 0x1FFFFFFF) | (1UL << 31);

  TSB_CAN->MC     |=  mbx_mask;           /* Enable mailbox                  */

  /* Setup data and length information                                       */
  if (msg->type == REMOTE_FRAME) {        /* For remote frame                */
    MailBox->DL     =  0;                 /* Data  low bytes                 */
    MailBox->DH     =  0;                 /* Data high bytes                 */
    MailBox->TSVMCF = (1 << 4) |          /* Set RTR bit                     */
                      (msg->len & 0x0F);  /* Set length reqested             */

  } else {                                /* For   data frame                */
    msg_data        = (unsigned int *) &msg->data[0];
    MailBox->DL     = *msg_data++;        /* Data  low bytes                 */
    MailBox->DH     = *msg_data;          /* Data high bytes                 */
    MailBox->TSVMCF = (msg->len & 0x0F);  /* Set length to transmit          */
  }

  TSB_CAN->TRS    |=  mbx_mask;           /* Set mailbox transmission request*/

  while (!(TSB_CAN->TA & mbx_mask));      /* Wait to transmit message        */
  TSB_CAN->TA = mbx_mask;                 /* Clear transmit flag             */

  return 0;
}

/*--------------------------- can_rx_object ---------------------------------*/

int can_rx_object (int ctrl, int ch, int id, int object_para)  {
  TSB_CANMB_TypeDef *MailBox  = table_mailbox_addr[ch-1];       /* Mbx adres */
  unsigned int   mbx_mask = (1 << (ch-1));

  TSB_CAN->MC     &= ~mbx_mask;           /* Disable mailbox                 */
  TSB_CAN->MD     |=  mbx_mask;           /* Set mailbox direction (rx)      */

  if ((object_para & FORMAT_TYPE) == STANDARD_TYPE)  { /* Standard identifier*/
    MailBox->ID     = ((id & 0x000007FF) << 18);
  } else {                                             /* Extended identifier*/
    MailBox->ID     = ((id & 0x1FFFFFFF) | (1UL << 31));
  }

  TSB_CAN->MC     |=  mbx_mask;           /* Enable mailbox                  */

  return 0;
}

/*--------------------------- can_tx_object ---------------------------------*/

int can_tx_object (int ctrl, int ch, int object_para)  {

  return 0;
}

