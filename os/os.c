#include "os.h"

#define UART0_BASE 0x101f1000

#define UART_DR      0x00  // Data Register
#define UART_FR      0x18  // Flag Register
#define UART_FR_TXFF 0x20  // Transmit FIFO Full
#define UART_FR_RXFE 0x10  // Receive FIFO Empty

volatile unsigned int * const UART0 = (unsigned int *)UART0_BASE;

void disable_watchdog(void){

}

void init_uart0(void){

}

void init_timer(void){

}

void init_intc(void){

}

void init_os(void){

}

void timer_irq_handler(void){

}


void uart_putc(char c){
    while (UART0[UART_FR / 4] & UART_FR_TXFF);
    UART0[UART_DR / 4] = c;
}

void uart_puts(const char *s){
    while (*s) {
        uart_putc(*s++);
    }
}
void uart_itoa(int val, char *buffer){
    int i = 0;
    int is_negative = 0;

    if (num == 0) {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return;
    }

    if (num < 0) {
        is_negative = 1;
        num = -num;
    }

    while (num > 0 && i < 14) { // Reserve space for sign and null terminator
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }

    if (is_negative) {
        buffer[i++] = '-';
    }

    buffer[i] = '\0';

    // Reverse the string
    int start = 0, end = i - 1;
    char temp;
    while (start < end) {
        temp = buffer[start];
        buffer[start] = buffer[end];
        buffer[end] = temp;
        start++;
        end--;
    }
}

// Funciones dummy (vacías) requeridas por tu stdio.c actual
void print_float(float val){
    if(f < 0){
        uart_putc('-');
        f =  -f;
    }

    int integer_part = (int)f;
    int decimal_part = (int)((f - integer_part)*100);
    char buffer[32];

    uart_itoa(integer_part, buffer);
    uart_puts(buffer);
    uart_putc('.');

    if (decimal_part < 10) uart_putc('0');
    uart_itoa(decimal_part, buffer);
    uart_puts(buffer);
}

float uart_atof(const char *s){
    float res = 0.0;
    int fact = 1;
    int i = 0;
    int sign = 1;

    if (s[i] == '-') {
        sign = -1;
        i++;
    }

    for (; s[i] != '\0' && s[i] != '.'; i++) {
        if (s[i] >= '0' && s[i] <= '9') {
            res = res * 10.0 + (s[i] - '0');
        }
    }

    if (s[i] == '.') {
        i++;
        for (; s[i] != '\0'; i++) {
            if (s[i] >= '0' && s[i] <= '9') {
                float decimal = (s[i] - '0');
                fact *= 10;
                res += decimal / fact;
            }
        }
    }
    return res * sign;
}

void uart_gets_input(char *buffer, int max_len){
    int i = 0;
    char c;
    while (i < max_length - 1) { // Leave space for null terminator
        c = uart_getc();
        if (c == '\n' || c == '\r') {
            uart_putc('\n'); // Echo newline
            break;
        }
        uart_putc(c); // Echo character
        buffer[i++] = c;
    }
    buffer[i] = '\0';
}

int uart_atoi(const char *str){
    int num = 0;
    int sign = 1;
    int i = 0;

    // Handle optional sign
    if (s[i] == '-') {
        sign = -1;
        i++;
    }

    for (; s[i] >= '0' && s[i] <= '9'; i++) {
        num = num * 10 + (s[i] - '0');
    }

    return sign * num;
}

float uart_atof(const char *str){

}