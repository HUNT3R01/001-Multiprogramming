#include "os.h"

// Envía un carácter por la UART emulada de QEMU.
void uart_putc(char c) {
    while (UART_FR & UART_FR_TXFF) {
    }
    UART_DR = (unsigned int)c;
}

// Lee un carácter desde la UART emulada.

char uart_getc(void) {
    while (UART_FR & UART_FR_RXFE) {
    }
    return (char)(UART_DR & 0xFF);
}

// Imprime una cadena completa.
void uart_puts(const char *s) {
    while (*s) {
        uart_putc(*s++);
    }
}

// Imprime un número decimal simple.
void uart_putnum(unsigned int num) {
    char buf[10];
    int i = 0;

    if (num == 0) {
        uart_putc('0');
        return;
    }

    while (num > 0) {
        buf[i++] = (char)((num % 10) + '0');
        num /= 10;
    }

    while (i > 0) {
        uart_putc(buf[--i]);
    }
}