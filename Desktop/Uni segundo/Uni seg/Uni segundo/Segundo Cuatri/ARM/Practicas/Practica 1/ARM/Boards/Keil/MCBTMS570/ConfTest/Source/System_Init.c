#include "system.h"

void System_Init(void);

void dev_init(void)
{
/*******************************************************************************************/
/*                       setup system registers                                            */
/*******************************************************************************************/
System_Init();
}

void System_Init(void)
{
 // Global reset to the peripheral
 SYSTEM_1->PENA=0;
 
 // Enabling the peripherals 
 SYSTEM_1->PENA=1;
 
 // Setting the Peripherals out of powerdown mode
 PCR->PSPWRDWNCLR0 = 0xffffffff;
 
 PCR->PSPWRDWNCLR1 = 0xffffffff;
 
 PCR->PSPWRDWNCLR2 = 0xffffffff;
 
 PCR->PSPWRDWNCLR3 = 0xffffffff;
 
}

void Peri_Reset()
{
 // Disabling the Peripheral
 SYSTEM_1->PENA=0;
 
}
