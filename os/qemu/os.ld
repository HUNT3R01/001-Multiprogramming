ENTRY(_start)

SECTIONS
{
    /* QEMU VersatilePB carga el kernel en 0x10000 */
    . = 0x10000;

    .text : {
        *(.text)
    }

    .data : {
        *(.data)
    }

    .bss : {
        __bss_start = .;
        *(.bss COMMON)
        __bss_end = .;
    }
}