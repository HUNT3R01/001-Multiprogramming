// user/P2/main.c
// Proceso 2 — imprime letras a-z en bucle infinito

extern void os_puts(const char *s);
extern void uart_putc(char c);

static void delay(void) {
    volatile int i;
    for (i = 0; i < 2000000; i++);
}

void p2_main(void) {
    char c = 'a';
    while (1) {
        os_puts("----From P2: ");
        uart_putc(c);
        os_puts("\r\n");
        c = (c == 'z') ? 'a' : c + 1;
        delay();
    }
}