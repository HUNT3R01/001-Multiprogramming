#include "os.h"

// Escritura 
void os_write(const char *s, int len) {
    int i;
    for (i = 0; i < len; i++) {
        uart_putc(s[i]);
    }
}

// Imprime una cadena completa por UART
void os_puts(const char *s) {
    while (*s) {
        uart_putc(*s++);
    }
}

// Lee una línea desde la UART de QEMU
void os_read_line(char *buffer, int max_len) {
    int i = 0;

    if (max_len <= 0) {
        return;
    }

    while (i < max_len - 1) {
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