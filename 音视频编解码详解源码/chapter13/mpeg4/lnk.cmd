MEMORY
{
	PMEM:	o = 0x00000000	l = 0x00030000
	DMEM:	o = 0x80000000	l = 0x20000000
}

SECTIONS
{
		.vectors	> PMEM
        .text		> DMEM
        .stack		> DMEM
        .bss		> DMEM
        .cinit      > DMEM
        .cio        > DMEM
        .const		> DMEM
        .data		> DMEM
        .switch		> DMEM
        .sysmem     > DMEM
        .far        > DMEM

}
