ENTRY(_start)

SECTIONS
{
    . = 0x10000;

    .text : {
        *(.text*)
        *(.rodata*)
    }

    . = ALIGN(4);

    .data : {
        *(.data*)
    }

    . = ALIGN(4);

    .bss : {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    }

    . = ALIGN(4);
}