
__asm void Set_MPU_Region (unsigned int region,unsigned int base_addr, unsigned int access, unsigned int size) {
	MCR p15, #0, r0, c6, c2, #0
	MCR p15, #0, r1, c6, c1, #0
	MCR p15, #0, r2, c6, c1, #4
	MCR p15, #0, r3, c6, c1, #2
  BX  lr
}

__asm void Enable_MPU(void) {
	MRC p15, #0, r1, c1, c0, #0 
	ORR r1, r1, #0x1
	DMB
	MCR p15, #0, r1, c1, c0, #0 
	ISB
	BX  lr
}

