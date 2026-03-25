// user/P1/main.c
// Proceso 1 — imprime digitos 0-9 en bucle infinito
// Usa funciones del OS via direcciones definidas en p1.ld

extern void os_puts(const char *s);
extern void uart_putc(char c);

static void delay(void) {
    volatile int i;
    for (i = 0; i < 2000000; i++);
}

void p1_main(void) {
    int n = 0;
    while (1) {
        os_puts("----From P1: ");
        uart_putc('0' + n);
        os_puts("\r\n");
        n = (n + 1) % 10;
        delay();
    }
}