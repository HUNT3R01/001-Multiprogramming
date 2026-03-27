// Proceso 1 — imprime digitos 0-9 en bucle infinito
#include "stdio.h"

static void delay(void);

void p1_main(void) {
    int n = 0;
    while (1) {
        PRINT("----From P1: %d\r\n", n);
        n = (n + 1) % 10;
        delay();
    }
}

static void delay(void) {
    volatile int i;
    for (i = 0; i < 100000000; i++);
}