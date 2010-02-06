ENTRY (loader)

SECTIONS{
    . = 0x00100000;

	.setup : {
		*(.setup)
	}

	. += 0xE0000000;

    .text : AT(ADDR(.text) - 0xE0000000) {
        *(.text)
    }

    .rodata ALIGN (0x1000) : AT(ADDR(.rodata) - 0xE0000000) {
        *(.rodata)
    }

    .data ALIGN (0x1000) : AT(ADDR(.data) - 0xE0000000) {
        *(.data)
    }

    .bss : AT(ADDR(.bss) - 0xE0000000) {
        sbss = .;
        *(COMMON)
        *(.bss)
        ebss = .;
    }

	end = .; _end = .; __end = .;
}
