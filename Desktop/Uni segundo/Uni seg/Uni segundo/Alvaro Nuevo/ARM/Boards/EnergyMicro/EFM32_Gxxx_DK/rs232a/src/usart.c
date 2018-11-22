/**************************************************************************//**
 * @file
 * @brief USART example
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
#include "efm32_cmu.h"
#include "usart.h"

#define RXBUFSIZE    8
static volatile int     rxReadIndex  = 0;
static volatile int     rxWriteIndex = 0;
static volatile int     rxCount      = 0;
static volatile uint8_t rxBuffer[RXBUFSIZE];

static uint8_t          CRtoCRLF = 0;

/**************************************************************************//**
 * @brief USART1 toggle CR to CRLF conversion
 * @param on If non-zero, automatic CR to CRLF conversion will be enabled
 *****************************************************************************/
void USART1_CRLF(int on)
{
  if (on)
    CRtoCRLF = 1;
  else
    CRtoCRLF = 0;
}

/**************************************************************************//**
 * @brief USART1 IRQ Handler
 *****************************************************************************/
void USART1_RX_IRQHandler(void)
{
  USART_TypeDef *usart = USART1;
  uint8_t       rxdata;

  if (usart->STATUS & USART_STATUS_RXDATAV)
  {
    rxdata = usart->RXDATA;
    /* Store Data */
    rxBuffer[rxWriteIndex] = rxdata;
    rxWriteIndex++;
    rxCount++;
    if (rxWriteIndex == RXBUFSIZE)
    {
      rxWriteIndex = 0;
    }
    /* Check for overflow */
    if (rxCount == RXBUFSIZE)
    {
      rxWriteIndex = 0;
      rxCount      = 0;
      rxReadIndex  = 0;
    }
  }
}

/**************************************************************************//**
 * @brief USART1 IRQ Setup
 *****************************************************************************/
static void USART1_IRQ_init(void)
{
  USART_TypeDef *usart = USART1;

  /* Clear previous interrupts */
  usart->IFC = _USART_IFC_MASK;
  /* Enable interrupts */
  usart->IEN = USART_IEN_RXDATAV;

  NVIC_ClearPendingIRQ(USART1_RX_IRQn);
  NVIC_EnableIRQ(USART1_RX_IRQn);
}

/**************************************************************************//**
 * @brief Receive a byte from USART1 and put into global buffer
 * @return -1 on failure, or positive character integer on sucesss
 *****************************************************************************/
int USART1_rxByte(void)
{
  int c = -1;

  __disable_irq();
  if (rxCount > 0)
  {
    c = rxBuffer[rxReadIndex];
    rxReadIndex++;
    if (rxReadIndex == RXBUFSIZE)
    {
      rxReadIndex = 0;
    }
    rxCount--;
  }
  __enable_irq();
  return c;
}


/**************************************************************************//**
 * @brief Transmit single byte to USART1
 * @param data Character to transmit
 *****************************************************************************/
int USART1_txByte(uint8_t data)
{
  USART_TypeDef *usart = USART1;

  /* Check that transmit buffer is empty */
  while (!(usart->STATUS & USART_STATUS_TXBL)) ;
  usart->TXDATA = (uint32_t) data;

  /* Add linefeed if enabled */
  if (CRtoCRLF && data == '\n')
  {
    while (!(usart->STATUS & USART_STATUS_TXBL)) ;
    usart->TXDATA = (uint32_t) '\r';
  }
  return (int) data;
}

/**************************************************************************//**
 * @brief Transmit buffer to USART1
 * @param buffer Array of characters to send
 * @param nbytes Number of bytes to transmit
 * @return Number of bytes sent
 *****************************************************************************/
int USART1_txBuf(uint8_t *buffer, int nbytes)
{
  int i;

  for (i = 0; i < nbytes; i++)
  {
    USART1_txByte(*buffer++);
  }
  return nbytes;
}

/**************************************************************************//**
 * @brief Intializes HFPERCLK for use with USART1
 *****************************************************************************/
static void USART1_CMU_init(void)
{
  CMU_TypeDef *cmu = CMU;

  /* Enable High Frequency Peripherals */
  cmu->HFPERCLKDIV |= CMU_HFPERCLKDIV_HFPERCLKEN;

  /* Enable USART 1 */
  cmu->HFPERCLKEN0 |= CMU_HFPERCLKEN0_USART1;
}

/**************************************************************************//**
 * @brief Intializes USART1
 *****************************************************************************/
void USART1_init(void)
{
  USART_TypeDef *usart = USART1;
  GPIO_TypeDef  *gpio  = GPIO;
  uint32_t      clk;

  /* Configure USART1 Clock */
  USART1_CMU_init();

  /* Configure USART1 - 8-N-1 */
  usart->FRAME &= (
    _USART_FRAME_DATABITS_MASK |
    _USART_FRAME_PARITY_MASK |
    _USART_FRAME_STOPBITS_MASK);
  usart->FRAME |= (
    USART_FRAME_DATABITS_EIGHT |
    USART_FRAME_PARITY_NONE |
    USART_FRAME_STOPBITS_ONE
    );

  /* Set baud rate clock divisor */
  /* 16x oversampling and 115200 baud rate. */
  /* Refer to ref.manual section 16.3.2.2 Clock Generation */
  clk = CMU_ClockFreqGet(cmuClock_USART1);
  usart->CLKDIV = (16 * clk) / (115200) - 256;

  /* Use default location 0 */
  /* 0 : TX - Pin C0, RX - Pin C1 */
  /* Configure GPIO pins LOCATION 0 as push pull (TX) */
  /* and input (RX) */
  /* To avoid false start, configure output as high   */
  gpio->P[2].DOUT   = (1 << 0);
  gpio->P[2].MODEL &= ~(
    _GPIO_P_MODEL_MODE0_MASK |
    _GPIO_P_MODEL_MODE1_MASK);
  gpio->P[2].MODEL |= GPIO_P_MODEL_MODE0_PUSHPULL;
  gpio->P[2].MODEL |= GPIO_P_MODEL_MODE1_INPUT;

  /* Enable default location - GPIO pins port C, pins 0 and 1 */
  usart->ROUTE = USART_ROUTE_RXPEN | USART_ROUTE_TXPEN;

  /* Clear RX/TX buffers */
  usart->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;

  /* Enable RX/TX */
  usart->CMD |= USART_CMD_RXEN | USART_CMD_TXEN;

  /* Set up receive interrupt */
  USART1_IRQ_init();
}
