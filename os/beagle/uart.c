#include "os.h"

// Envía un carácter por UART0.
void uart_putc(char c) {
    while ((GET32(UART_LSR) & UART_LSR_TX) == 0) {
    }
    PUT32(UART_THR, (unsigned int)c);
}

// Lee un carácter recibido por UART0.
char uart_getc(void) {
    while ((GET32(UART_LSR) & UART_LSR_RX) == 0) {
    }
    return (char)(GET32(UART_THR) & 0xFF);
}

// Imprime un número decimal .
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