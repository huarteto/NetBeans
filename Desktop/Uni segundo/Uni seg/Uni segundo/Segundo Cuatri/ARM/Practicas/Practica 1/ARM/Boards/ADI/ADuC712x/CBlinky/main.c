#include <stdio.h>
#include <ADuC7128.H>

//*** <<< Use Configuration Wizard in Context Menu >>> *** 
//  

///////////////////////////////////////////////////////
// Filename:    main.c
// Author:      ADI
// Date:        13/12/2006
// Description: Blinks led(s) on ADuC7128 eval board
//              Prints message out to the UART
// 
//              C Program that blinks the LED(s)
//              Can be configured to use 
//               1) Interrupts
//               2) Delay Loops
//               3) Timer Delays
// 
///////////////////////////////////////////////////////

// Constants
#define WAKEUP_TIMER_BIT         0x00000010                 
#define DIR_VAL                  0xFF000000

void  InterruptIRQHndlr   (void);

static unsigned long ulLoop   = 0x0;

// Function    : Delay
// Author      : ADI
// Description : main
// Revision    : 13.12.2005 - Initial
void Delay(void)
{
   volatile int iDelay = 0x1FFFF;

   while (iDelay--);
}

// Function    : LedToggle
// Author      : ADI
// Description : Advance leds based on timer interrupt
// Revision    : 13.12.2005 - Initial
void  LedToggle(void)
{
   // Apps EvalBoard with P4.2 to blink
   if (ulLoop%2)
	  GP4SET = 0x00040000;
   else
	  GP4CLR = 0x00040000;

   ulLoop++;
}

// Function    : InterruptIRQHndlr
// Author      : ADI
// Description : Advance leds based on timer interrupt
// Revision    : 13.12.2005 - Initial
void  InterruptIRQHndlr   (void)
{  
   if ((IRQSTA & WAKEUP_TIMER_BIT) != 0)      // Timer2 IRQ?
      {
      LedToggle();
      T2ICLR = 0x0; // Clear the interrupt
      }
}

// Function    : ConfigureTimer2
// Description : ConfigureTimer2
// Revision    : 13.12.2005 - Initial
void ConfigureTimer2(void)
{
   // Initialise
   IRQCLR      = WAKEUP_TIMER_BIT;  // Timer2 IRQ
   IRQ         = InterruptIRQHndlr; // Specify Interrupt Service Rountine
   T2LD        = 0x1000;      
   T2CON       = 0x2C0;                                 
   IRQEN       = WAKEUP_TIMER_BIT;    // Timer2 IRQ
}

// Function    : ConfigureUART9600
// Author      : ADI
// Description : Configure the UART for 9600 baud
// Revision    : 13.12.2005 - Initial
void ConfigUART9600(void)
{
   unsigned long  ul9600BaudDiv;
   unsigned long Div = POWCON & 0x3;

	// Setup tx & rx pins on P1.0 and P1.1
   GP1CON = 0x11;

   // Start will a full speed divisor and adjust for POWCON.CD
   ul9600BaudDiv = 136;	// 41 Mhz
   while(Div--)
      ul9600BaudDiv /= 0x2;

   COM0CON0 = 0x80;
   COM0DIV0 = ul9600BaudDiv;// 9600 baud scaled by POWCON value
   COM0DIV1 = 0x0;
   COM0CON0 = 0x3;
   COM0DIV2 = 0x0;
}

// Function    : main
// Author      : ADI
// Description : main
// Revision    : 13.12.2005 - Initial
int main(void)
{   
   GP4CON = 0x0;
   GP4DAT = DIR_VAL;

   // Configure the UART before calling printf.
   ConfigUART9600();

   printf("\n\nADuC712x RealView Example Program\n");

   // Toggle the leds based on timer interrupt
   ConfigureTimer2();
   while (1) ; // loop
}

