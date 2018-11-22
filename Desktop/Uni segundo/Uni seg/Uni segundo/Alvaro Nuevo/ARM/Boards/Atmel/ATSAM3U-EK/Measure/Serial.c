/*----------------------------------------------------------------------------
 * Name:    Serial.c
 * Purpose: AT91SAM3U-EK Low level serial functions
 * Version: V1.00
 * Note(s):
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2009 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------
 * History:
 *          V1.00 Initial Version
 *----------------------------------------------------------------------------*/

#include <SAM3U.H>                                   /* ATSAM3U definitions   */
#include "Serial.h"

/* Following definitions can be used to select the used communication interface:

  _DBGU_     - DBGU interface  used
  _USART1_   - COM1 interface  used  (default)   */
//#define _DBGU_

/* Clock Definitions */
#define BAUD(b) ((SystemFrequency + 8*b)/(16*b))

/*----------------------------------------------------------------------------
  Initialize UART pins, Baudrate  (currently only DBGU is used)
 *----------------------------------------------------------------------------*/
void SER_init (void) {
#ifdef _DBGU_
  PMC->PMC_PCER = ((1UL << ID_PIOA) |        /* enable PIOA clock */
                   (1UL << ID_UART)  );      /* enable UART clock */  

  /* Configure UART for 115200 baud. */
  PIOA->PIO_IDR    =
  PIOA->PIO_PUDR   =
  PIOA->PIO_PDR    =  (PIO_PA11A_URXD | PIO_PA12A_UTXD);
  PIOA->PIO_ABSR  &= ~(PIO_PA11A_URXD | PIO_PA12A_UTXD);

  UART->UART_CR    = UART_CR_RSTRX   | UART_CR_RSTTX;
  UART->UART_IDR   = 0xFFFFFFFF;
  UART->UART_BRGR  = BAUD(115200);
  UART->UART_MR    = (0x4 <<  9);            /* (DBGU) No Parity */
  UART->UART_PTCR  = UART_PTCR_RXTDIS | UART_PTCR_TXTDIS;
  UART->UART_CR    = UART_CR_RXEN     | UART_CR_TXEN;
#else
  PMC->PMC_PCER = ((1UL << ID_PIOA  ) |      /* enable PIOA   clock */
                   (1UL << ID_USART1)  );    /* enable USART1 clock */  

  /* Configure DBGU for 115200 baud. */
  PIOA->PIO_IDR    =
  PIOA->PIO_PUDR   =
  PIOA->PIO_PDR    =  (PIO_PA21A_RXD1 | PIO_PA20A_TXD1);
  PIOA->PIO_ABSR  &= ~(PIO_PA21A_RXD1 | PIO_PA20A_TXD1);

  USART1->US_CR    = US_CR_RSTRX   | US_CR_RSTTX;
  USART1->US_IDR   = 0xFFFFFFFF;
  USART1->US_BRGR  = BAUD(115200);
  USART1->US_MR    = (0x0 <<  0) |           /* (USART) Normal */
                     (0x0 <<  4) |           /* (USART) Clock */
                     (0x3 <<  6) |           /* (USART) Character Length: 8 bits */
                     (0x4 <<  9) |           /* (DBGU) No Parity */
                     (0x0 << 12) |           /* (USART) 1 stop bit */
                     (0x0 << 14) ;           /* (DBGU) Normal Mode: The USART channel operates as an RX/TX USART. */
  USART1->US_PTCR  = US_PTCR_RXTDIS | US_PTCR_TXTDIS;
  USART1->US_CR    = US_CR_RXEN     | US_CR_TXEN;
#endif
}


/*----------------------------------------------------------------------------
  check if character is ready at serial port
 *----------------------------------------------------------------------------*/
int SER_charReady (void) {               /* character is ready at serial port */
#ifdef _DBGU_
  return (UART->UART_SR  & UART_SR_RXRDY);
#else
  return (USART1->US_CSR & US_CSR_RXRDY);
#endif
}

/*----------------------------------------------------------------------------
  Write character to Serial Port
 *----------------------------------------------------------------------------*/
int SER_putChar (int c) {
#ifdef _DBGU_
   while (!(UART->UART_SR  & UART_SR_TXRDY));
   UART->UART_THR = c;
#else
   while (!(USART1->US_CSR & US_CSR_TXRDY));
   USART1->US_THR = c;
#endif
   return (c);
}


/*----------------------------------------------------------------------------
  Read character from Serial Port   (blocking read)
 *----------------------------------------------------------------------------*/
int SER_getChar (void) {
#ifdef _DBGU_
  while (!(UART->UART_SR  & UART_SR_RXRDY));
  return (UART->UART_RHR);
#else
  while (!(USART1->US_CSR & US_CSR_RXRDY));
  return (USART1->US_RHR);
#endif
}


/*----------------------------------------------------------------------------
  Read character from Serial Port   (non blocking read)
 *----------------------------------------------------------------------------*/
int SER_getChar_nb (void) {
#ifdef _DBGU_
  if (UART->UART_SR  & UART_SR_RXRDY)
    return (UART->UART_RHR);
#else
  if (USART1->US_CSR & US_CSR_RXRDY)
    return (USART1->US_RHR);
#endif
  else
    return 0;
}


/*----------------------------------------------------------------------------
  Write character to Serial Port
 *----------------------------------------------------------------------------*/
void SER_putString (unsigned char *s) {

  while (*s != 0) {
   SER_putChar(*s++);
  }
}
