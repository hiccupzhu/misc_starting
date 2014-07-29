
MEMORY
{
	ISRAM:	o = 0x00001000	l = 0x00020000
	SDRAM:	o = 0x80000000	l = 0x02000000
}

SECTIONS
{
        .text		> ISRAM
        .stack		> SDRAM
        .bss		> SDRAM
        .cinit      > SDRAM
        .cio        > SDRAM
        .const		> SDRAM
        .data		> SDRAM
        .switch		> SDRAM
        .sysmem     > SDRAM
        .far        > SDRAM
}


-l cslDM642.lib
-l rts6400.lib          /* C and C++ run-time library support */
-l img64x.lib

