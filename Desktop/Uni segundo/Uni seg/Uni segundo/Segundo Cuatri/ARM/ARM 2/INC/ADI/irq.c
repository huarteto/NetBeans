/*    Function Pointers for Interrupts  */
typedef void (* tyVctHndlr) (void);

tyVctHndlr    IRQ     = (tyVctHndlr)0x0;
tyVctHndlr    SWI     = (tyVctHndlr)0x0;
tyVctHndlr    FIQ     = (tyVctHndlr)0x0;
tyVctHndlr    UNDEF   = (tyVctHndlr)0x0;
tyVctHndlr    PABORT  = (tyVctHndlr)0x0;
tyVctHndlr    DABORT  = (tyVctHndlr)0x0;

void	ADI_IRQ_Interrupt_Setup(void) __attribute__ ((interrupt ("IRQ")));
void	ADI_SWI_Interrupt_Setup(void) __attribute__ ((interrupt ("SWI")));
void	ADI_FIQ_Interrupt_Setup(void) __attribute__ ((interrupt ("FIQ")));
void	ADI_UNDEF_Interrupt_Setup(void) __attribute__ ((interrupt ("UNDEF")));
void	ADI_PABORT_Interrupt_Setup(void) __attribute__ ((interrupt ("ABORT")));
void	ADI_DABORT_Interrupt_Setup(void)  __attribute__ ((interrupt ("ABORT")));

void	ADI_IRQ_Interrupt_Setup(void) 
{
	if ( *IRQ !=0x00)
	{
		IRQ();
	}
}

void	ADI_FIQ_Interrupt_Setup(void) 
{
	if ( *FIQ !=0x00)
	{
		FIQ();
	}
}

void	ADI_SWI_Interrupt_Setup(void) 
{
	if ( *SWI !=0x00)
	{
		SWI();
	}
}

void	ADI_UNDEF_Interrupt_Setup(void) 
{
	if ( *UNDEF !=0x00)
	{
		UNDEF();
	}
}

void	ADI_PABORT_Interrupt_Setup(void) 
{
	if ( *PABORT !=0x00)
	{
		PABORT();
	}
}

void	ADI_DABORT_Interrupt_Setup(void) 
{
	if ( *DABORT !=0x00)
	{
		DABORT();
	}
}

