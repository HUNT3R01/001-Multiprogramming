#include "stdio.h"
#include "os.h"
#include <stdarg.h>

void PRINTF(const char *format, ...) {
    va_list args;
    va_start(args, format);

    while (*format != '\0') {
        if (*format == '%') {
            format++;
            
            switch (*format) {
                case 'd': {
                    int val = va_arg(args, int);
                    char buffer[32];
                    uart_itoa(val, buffer);
                    uart_puts(buffer);
                    break;
                }
                case 's': {
                    char *s = va_arg(args, char *); 
                    uart_puts(s);
                    break;
                }
                case 'f': {
                    float val = (float)va_arg(args, double);
                    print_float(val);
                    break;
                }
                default:
                    uart_putc('%');
                    uart_putc(*format);
                    break;
            }
        } else {
            uart_putc(*format);
        }
        format++;
    }
    va_end(args);
}

void READ(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char input_buffer[100];

    while (*format != '\0') {
        if (*format == '%') {
            format++;
            switch (*format) {
                case 'd': {
                    int *p = va_arg(args, int *);
                    uart_gets_input(input_buffer, sizeof(input_buffer));
                    *p = uart_atoi(input_buffer);
                    break;
                }
                case 's': {
                    char *p = va_arg(args, char *);
                    uart_gets_input(p, 100);
                    break;
                }
                case 'f': {
                    float *p = va_arg(args, float *);
                    uart_gets_input(input_buffer, sizeof(input_buffer));
    
                    *p = uart_atof(input_buffer);
                    break;
                }
            }
        }
        format++;
    }
    va_end(args);
}