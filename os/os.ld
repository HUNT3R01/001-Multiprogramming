/* os/os.ld */
ENTRY(_start) /* Le dice al compilador que el programa empieza en _start (en root.s) */

SECTIONS
{
    /* El bloque OS comienza en esta dirección exacta de memoria */
    . = 0x82000000;

    /* Sección de código (.text) */
    .text : {
        *(.text)
    }

    /* Sección de datos inicializados (.data) */
    .data : {
        *(.data)
    }

    /* Sección de datos NO inicializados (.bss) */
    .bss : {
        __bss_start__ = .;  /* Marca el inicio de .bss para que root.s lo limpie */
        *(.bss)
        *(COMMON)
        __bss_end__ = .;    /* Marca el final de .bss */
    }
}