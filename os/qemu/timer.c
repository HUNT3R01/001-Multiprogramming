#include "os.h"

// Inicializa el timer SP804 en QEMU y habilita su IRQ.
void timer_init(void) {
    TIMER0_LOAD = TIMER_LOAD_VAL;

    // Enable | Periodic | Interrupt enable | 32-bit
    TIMER0_CONTROL = (1 << 7) | (1 << 6) | (1 << 5) | (1 << 1);

    VIC_INTENABLE = TIMER0_IRQ_BIT;
}