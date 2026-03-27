// Proceso 2 — imprime letras a-z en bucle infinito
#include "stdio.h"

static void delay(void);

void p2_main(void) {
    char c = 'a';
    while (1) {
        PRINT("----From P2: %c\r\n", c);
        c = (c == 'z') ? 'a' : c + 1;
        delay();
    }
}

static void delay(void) {
    volatile int i;
    for (i = 0; i < 100000000; i++);
}