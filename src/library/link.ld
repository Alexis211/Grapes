ENTRY (start)
INPUT (grapes.o)
 
SECTIONS{
    . = 0x100000;
 
    .text : {
        *(.text)
    }
 
    .rodata ALIGN (0x1000)  :{
        *(.rodata)
    }
 
    .data ALIGN (0x1000) : {
        *(.data)
    }
 
    .bss : {
        sbss = .;
        *(COMMON)
        *(.bss)
        ebss = .;
    }
 
	end = .; _end = .; __end = .;
}
