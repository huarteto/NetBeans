//**********************************************************************
// This code was created by SoC Solutions and is free for use with     *
// Actel products.  For support e-mail support@socsolutions.com.       *
//**********************************************************************

// Included files
#include "socDefs.h"
#include "APB_Interrupt.h"
#include "APB_Timer.h"
#include "APB_UART.h"
#include "platform.h"
#include "externals.h"

// Defines and Macros
#define TIMER_PRESCALE          TIMER_PRESCALE_DIV2

#define TIMER_TIMEOUT_1_00_S    ((CLK_FREQ_HZ)>>((TIMER_PRESCALE)+1))
#define TIMER_TIMEOUT_2_00_S    (TIMER_TIMEOUT_1_00_S<<1)
#define TIMER_TIMEOUT_0_12_S    (TIMER_TIMEOUT_1_00_S>>3)

#define TIMEOUT_MIN             TIMER_TIMEOUT_0_12_S
#define TIMEOUT_MAX             TIMER_TIMEOUT_2_00_S
#define TIMEOUT_SCALE(x)        (TIMEOUT_MIN+(((x)*(TIMEOUT_MAX - TIMEOUT_MIN))>>7))

// This can be coded as a macro if it is desired to use standard libraries necessary for the division.
//#define BR_57600_16MHZ (CLK_FREQ_HZ/((57600+1)<<4)) + ((CLK_FREQ_HZ%((57600+1)<<4)<<1)/(CLK_FREQ_HZ/((57600+1)<<4)))
#define BR_57600_16MHZ          17


// Global Variables

// This value governs the frequency at which the code in the main() loop is executed.
// It is set upon a UART Receive interrupt, which occurs if the PC application is connected.
unsigned glblSpeedVal = 64;

// This value is incremented upon a timer tick interrupt.
unsigned glblTimerCount = 0;

// The interrupt service routine
void IRQ_Handler (void)
{
    unsigned source;

    source = interrupt_irq_status(INTERRUPT_BASE_ADDR);

    if (source & IRQ_UART_RX_MASK)
    {
        // Reading the receive data register clears the interrupt
        glblSpeedVal = uart_getchar(UART_BASE_ADDR);
    }

    if (source & IRQ_TIMER_TICK_MASK)
    {
        // Writing the interrupt clear register clears the interrupt
        timer_irq_clear(TIMER_BASE_ADDR);
        glblTimerCount++;
    }
}

// The main function
int main( void )
{
    unsigned char m_Lstate, m_Sstate;

    // Configure UART to operate at 57600
    uart_cfg(UART_BASE_ADDR, BR_57600_16MHZ);

    // Configure Timer: Continuous mode, IRQ Enable, Timer Enable, Prescale as defined above
    timer_load(TIMER_BASE_ADDR, TIMEOUT_SCALE(glblSpeedVal));
    timer_cfg(TIMER_BASE_ADDR, TIMER_CONTROL_CONTINUOUS | TIMER_CONTROL_IRQENABLE | TIMER_CONTROL_ENABLE, TIMER_PRESCALE);

    // Enable interrupts
    interrupt_irq_enable(INTERRUPT_BASE_ADDR, IRQ_UART_RX_MASK | IRQ_TIMER_TICK_MASK);
    cm1_irq_enable(CM1_IRQ0_MASK);

    // Initialize m_Sstate
    m_Sstate = 0;

    // The main (forever) loop
    for(;;)
    {
		// Create a sequence...
        switch (m_Sstate)
        {
            case 0 :
				m_Lstate = 'K'; // Red / Don't Walk
				break;
			case 1 :
			case 2 :
			case 3 :
			case 4 :
			case 5 :
			case 6 :
				m_Lstate = 'E'; // Green / Don't Walk
				break;
			case 7 :
			case 8 :
				m_Lstate = 'H'; // Yellow / Don't Walk
				break;
			case 9 :
				m_Lstate = 'K'; // Red / Don't Walk
				break;
			case 10 :
			case 11 :
			case 12 :
				m_Lstate = 'L'; // Red / Walk
				break;
			case 13 :
				m_Lstate = 'K'; // Red / Don't Walk
				break;
			case 14 :
				m_Lstate = 'J'; // Red / Off
				break;
			case 15 :
			    m_Lstate = 'K'; // Red / Don't Walk
			    break;
			case 16 :
				m_Lstate = 'J'; // Red / Off
				break;
			default :
				m_Lstate = 'K'; // Red / Don't Walk
				break;
        }

        // Output to the GUI (if connected) via the UART
        uart_puts(UART_BASE_ADDR, &m_Lstate, 1);
        // Output to the LEDS
        gpio_out(GPIO_BASE_ADDR, m_Lstate);

        // Wait for the timer to expire
        while ( *((volatile unsigned *) &glblTimerCount) == 0 );        
        glblTimerCount = 0;

        // Reprogram the Timer with the latest speed update (if any) from the GUI
        // If GUI is connected, there may be a speed update available.
        timer_load(TIMER_BASE_ADDR, TIMEOUT_SCALE(glblSpeedVal));

        // Increment m_Sstate
        if (m_Sstate >= 16) m_Sstate = 0;
        else                m_Sstate++;

    }
}

