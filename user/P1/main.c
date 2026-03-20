// Proceso 1 — imprime digitos 0 al 9 en bucle infinito
// Usamos las funciones de UART del OS directamente
extern void os_puts(const char *s);
extern void uart_putc(char c);

// Delay simple para que la salida sea legible
static void delay(void) {
    volatile int i;
    for (i = 0; i < 2000000; i++);
}

// Convierte un digito entero a caracter y lo imprime
static void print_digit(int n) {
    uart_putc('0' + n);
}

void p1_main(void) {
    int n = 0;
    while (1) {
        os_puts("----From P1: ");
        print_digit(n);
        os_puts("\r\n");
        n = (n + 1) % 10;
        delay();
    }
}