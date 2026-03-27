ENTRY(p1_main)

SECTIONS
{
    . = 0x82100000;

    .text : { *(.text) *(.rodata) }

    . = ALIGN(4);
    .data : { *(.data) }

    . = ALIGN(4);
    .bss : {
        __p1_bss_start__ = .;
        *(.bss) *(COMMON)
        __p1_bss_end__ = .;
    }

    . = 0x82110000;
    _p1_stack_bottom = .;
    . = . + 0x2000;
    _p1_stack_top = .;
}

