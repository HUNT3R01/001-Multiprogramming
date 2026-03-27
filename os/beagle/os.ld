/* Linker script del OS para BeagleBone Black */
ENTRY(_start)

SECTIONS
{
    /* Dirección base donde se carga el OS */
    . = 0x82000000;

    /* Código y constantes */
    .text :
    {
        *(.text*)
        *(.rodata*)
    }

    . = ALIGN(4);

    /* Datos inicializados */
    .data :
    {
        *(.data*)
    }

    . = ALIGN(4);
    __bss_start__ = .;

    /* Datos no inicializados */
    .bss :
    {
        *(.bss*)
        *(COMMON)
    }

    . = ALIGN(4);
    __bss_end__ = .;
}