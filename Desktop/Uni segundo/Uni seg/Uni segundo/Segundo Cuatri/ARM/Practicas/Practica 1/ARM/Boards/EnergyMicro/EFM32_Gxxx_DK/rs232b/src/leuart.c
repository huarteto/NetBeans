/**************************************************************************//**
 * @file
 * @brief LEUART example
 * @author Energy Micro AS
 * @version 1.2.0
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2009 Energy Micro AS, http://www.energymicro.com</b>
 ******************************************************************************
 *
 * This source code is the property of Energy Micro AS. The source and compiled
 * code may only be used on Energy Micro "EFM32" microcontrollers.
 *
 * This copyright notice may not be removed from the source code nor changed.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Energy Micro AS has no
 * obligation to support this Software. Energy Micro AS is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Energy Micro AS will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 *****************************************************************************/
#include "efm32.h"
#include "leuart.h"

#define RXBUFSIZE    8
static volatile int     rxReadIndex  = 0;
static volatile int     rxWriteIndex = 0;
static volatile int     rxCount      = 0;
static volatile uint8_t rxBuffer[RXBUFSIZE];

static uint8_t CRtoCRLF = 0;

/**************************************************************************//**
 * @brief LEUART1 toggle CR to CRLF conversion
 * @param on If non-zero, automatic CR to CRLF conversion will be enabled
 *****************************************************************************/
void LEUART1_CRLF(int on)
{
  if (on) 
    CRtoCRLF = 1;
  else 
    CRtoCRLF = 0;
}


/**************************************************************************//**
 * @brief LEUART1 interrupt handler
 *****************************************************************************/
void LEUART1_IRQHandler(void)
{
  LEUART_TypeDef *leuart = LEUART1;
  uint8_t data;

  if (leuart->IF & LEUART_IF_RXDATAV)
  {
    /* Store data */
    data = leuart->RXDATA;
    rxBuffer[rxWriteIndex] = data;
    rxWriteIndex++;
    rxCount++;
    /* Check wrap around on buffer */
    if (rxWriteIndex == RXBUFSIZE)
    {
      rxWriteIndex = 0;
    }
    /* Check overflow */
    if (rxCount == RXBUFSIZE)
    {
      /* Flush data on overflow */
      rxWriteIndex = 0;
      rxReadIndex  = 0;
      rxCount      = 0;
    }
  }
  leuart->IFC = _LEUART_IFC_MASK; /* Clear all interrupts */
}

/**************************************************************************//**
 * @brief Intializes LEUART1 interrupt on RX (receive)
 *****************************************************************************/
static void LEUART1_IRQ_init(void)
{
  LEUART_TypeDef *leuart = LEUART1;

  /* Clear all pending interrupts */
  leuart->IFC = _LEUART_IFC_MASK;
  /* Enable desired LEUART interrupts */
  leuart->IEN = LEUART_IEN_RXDATAV;

  /* Enable interrupt in Cortex Core */
  NVIC_ClearPendingIRQ(LEUART1_IRQn);
  NVIC_EnableIRQ(LEUART1_IRQn);

}

/**************************************************************************//**
 * @brief Transmit single byte to LEUART1
 * @param data Character to transmit
 *****************************************************************************/
int LEUART1_txByte(uint8_t data)
{
  LEUART_TypeDef *leuart = LEUART1;

  /* Check that transmit buffer is empty */
  while (!(leuart->STATUS & LEUART_STATUS_TXBL)) ;
  leuart->TXDATA = (uint32_t) data;
  if ( CRtoCRLF && data == '\n' ) 
  {
    while (!(leuart->STATUS & LEUART_STATUS_TXBL)) ;
    leuart->TXDATA = '\r';
  }
  return (int) data;
}

/**************************************************************************//**
 * @brief Transmit buffer to LEUART1
 * @param buffer Array of characters to send
 * @param nbytes Number of bytes to transmit
 * @return Number of bytes sent
 *****************************************************************************/
int LEUART1_txBuf(uint8_t *buffer, int nbytes)
{
  int i;

  for (i = 0; i < nbytes; i++)
  {
    LEUART1_txByte(*buffer++);
  }
  return nbytes;
}


/**************************************************************************//**
 * @brief Receive a number of bytes from LEUART1
 * @param buffer Array for received characters
 * @param nbytes Number of bytes to read
 * @return Number of bytes received
 *****************************************************************************/
int LEUART1_rxBuf(uint8_t *buffer, int nbytes)
{
  int i, data;

  for (i = 0; i < nbytes; i++)
  {
    data = LEUART1_rxByte();
    if (data > 0)
    {
      *buffer++ = data;
    }
  }
  return nbytes;
}

/**************************************************************************//**
 * @brief Receive a number of bytes from LEUART1
 * @return -1 on failure, or positive character integer on sucesss
 *****************************************************************************/
int LEUART1_rxByte(void)
{
  int retValue = -1;

  /* Protect region for simultaneous interrupt handler updates */
  __disable_irq();
  if (rxCount > 0)
  {
    retValue = (int) rxBuffer[rxReadIndex];
    rxReadIndex++;
    if (rxReadIndex == RXBUFSIZE)
    {
      rxReadIndex = 0;
    }
    rxCount--;
  }
  /* Core CMSIS portable interrupt enable/disable */
  __enable_irq();

  return retValue;
}

/**************************************************************************//**
 * @brief Intializes LFXO as LFBCLK for use with LEUART1
 *****************************************************************************/
static void LEUART1_CMU_init(void)
{
  CMU_TypeDef *cmu = CMU;

  /* Setup and enable LEUART clock */
  cmu->HFCORECLKEN0 |= CMU_HFCORECLKEN0_LE;
  /* Enable LFXO */
  /* Note: LFXO is precalibrated on the kit, we could also use LFRCO here */
  cmu->OSCENCMD |= CMU_OSCENCMD_LFXOEN;

  /* Wait until LFXO clock is ready */
  while (!(cmu->STATUS & CMU_STATUS_LFXORDY)) ;

  /* Select LFXO as clock source for LFBCLK */
  cmu->LFCLKSEL &= ~(_CMU_LFCLKSEL_LFB_MASK);
  cmu->LFCLKSEL |= (CMU_LFCLKSEL_LFB_LFXO);

  /* Set prescale for LEUART1 */
  cmu->LFBPRESC0 = 0;

  /* Enable LFBCLK for LEUART1 */
  cmu->LFBCLKEN0 |= CMU_LFBCLKEN0_LEUART1;

  /* Wait until all settled */
  while (cmu->SYNCBUSY & CMU_SYNCBUSY_LFBCLKEN0) ;
}

/**************************************************************************//**
 * @brief Intializes LEUART1 for use as an output interface
 * @param baudrate 300 - 9600 baud
 * @param databits 7 or 8 data bits
 * @param parity 0 = No parity, 01 = Resrvd, 02 = Even parity, 03 = Odd parity
 * @param stopbits 0 or 1 stop bits
 *****************************************************************************/
void LEUART1_init(int baudrate, int databits, int parity, int stopbits)
{
  GPIO_TypeDef   *gpio   = GPIO;
  LEUART_TypeDef *leuart = LEUART1;
  uint32_t       clkdiv;

  /* Configure LEUART1 LFBCL */
  LEUART1_CMU_init();

  /* Configure LEUART1 */

  /* Set baud rate - LFXO clock rate is 32768Hz on kit */
  clkdiv         = ((256 * 32768) / baudrate - 256);
  leuart->CLKDIV = clkdiv;

  /* Set data bits */
  leuart->CTRL &= ~(_LEUART_CTRL_DATABITS_MASK);
  if (databits == 9)
  {
    leuart->CTRL |= LEUART_CTRL_DATABITS_NINE;
  }
  else
  {
    leuart->CTRL |= LEUART_CTRL_DATABITS_EIGHT;
  }

  /* Set parity */
  leuart->CTRL &= ~(_LEUART_CTRL_PARITY_MASK);
  switch (parity)
  {
  case 3:
    leuart->CTRL |= LEUART_CTRL_PARITY_ODD;
    break;
  case 2:
    leuart->CTRL |= LEUART_CTRL_PARITY_EVEN;
    break;
  default:
    leuart->CTRL |= LEUART_CTRL_PARITY_NONE;
    break;
  }

  /* Set stop bits */
  leuart->CTRL &= ~(_LEUART_CTRL_STOPBITS_MASK);
  if (stopbits == 2)
  {
    leuart->CTRL |= LEUART_CTRL_STOPBITS_TWO;
  }
  else
  {
    leuart->CTRL |= LEUART_CTRL_STOPBITS_ONE;
  }
  while (leuart->SYNCBUSY) ;

  /* Configure GPIO pins LOCATION 0 as push pull (TX) */
  /* and input (RX) */
  /* To avoid false start, configure output as high   */
  /* LEUART1 primary locations for EFM32G890F128 are */
  /* 0 : TX - Pin C6, RX - Pin C7 */
  gpio->P[2].DOUT   = (1 << 6);
  gpio->P[2].MODEL &= ~(
    _GPIO_P_MODEL_MODE6_MASK|
    _GPIO_P_MODEL_MODE7_MASK);
  gpio->P[2].MODEL |= GPIO_P_MODEL_MODE6_PUSHPULL;
  gpio->P[2].MODEL |= GPIO_P_MODEL_MODE7_INPUT;

  /* Enable default location - which is GPIO pins over */
  leuart->ROUTE &= ~(_LEUART_ROUTE_LOCATION_MASK);
  leuart->ROUTE |= LEUART_ROUTE_LOCATION_LOC0;
  leuart->ROUTE |= LEUART_ROUTE_TXPEN | LEUART_ROUTE_RXPEN;

  /* Clear RX/TX buffers */
  leuart->CMD = LEUART_CMD_CLEARRX | LEUART_CMD_CLEARTX;

  /* Enable RX/TX */
  leuart->CMD |= LEUART_CMD_RXEN | LEUART_CMD_TXEN;
  while (leuart->SYNCBUSY) ;

  /* Configure interrupt handler */
  LEUART1_IRQ_init();
}
