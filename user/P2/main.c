// user/P2/main.c
// Proceso 2 — imprime letras a-z en bucle infinito

static void call_os_puts(const char *s) {
    typedef void (*fn_t)(const char *);
    ((fn_t)0x82000320)(s);
}

static void call_uart_putc(char c) {
    typedef void (*fn_t)(char);
    ((fn_t)0x82000120)(c);
}

static void delay(void) {
    volatile int i;
    for (i = 0; i < 2000000; i++);
}

void p2_main(void) {
    char c = 'a';
    while (1) {
        call_os_puts("----From P2: ");
        call_uart_putc(c);
        call_os_puts("\r\n");
        c = (c == 'z') ? 'a' : c + 1;
        delay();
    }
}