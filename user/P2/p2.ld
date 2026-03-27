ENTRY(p2_main)

SECTIONS
{
    . = 0x82200000;

    .text : { *(.text) *(.rodata) }

    . = ALIGN(4);
    .data : { *(.data) }

    . = ALIGN(4);
    .bss : {
        __p2_bss_start__ = .;
        *(.bss) *(COMMON)
        __p2_bss_end__ = .;
    }

    . = 0x82210000;
    _p2_stack_bottom = .;
    . = . + 0x2000;
    _p2_stack_top = .;
}
