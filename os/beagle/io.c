#include "os.h"

//Escritura 
void os_write(const char *s, unsigned int len) {
    unsigned int i;
    for (i = 0; i < len; i++) {
        uart_putc(s[i]);
    }
}

// Imprime una cadena completa.
void os_puts(const char *s) {
    while (*s) {
        uart_putc(*s++);
    }
}

// Lee una línea desde UART.
void os_read_line(char *buffer, int max_length) {
    int i = 0;

    while (i < max_length - 1) {
        char c = uart_getc();

        if (c == '\r' || c == '\n') {
            uart_putc('\r');
            uart_putc('\n');
            break;
        }

        if (c == '\b' || c == 127) {
            if (i > 0) {
                i--;
                uart_putc('\b');
                uart_putc(' ');
                uart_putc('\b');
            }
            continue;
        }

        buffer[i++] = c;
        uart_putc(c);
    }

    buffer[i] = '\0';
}