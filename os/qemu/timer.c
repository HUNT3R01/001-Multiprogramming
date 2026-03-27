#include "os.h"

// Convierte el quantum en milisegundos al valor de cargadel timer SP804.
// En QEMU el timer se carga directamente con los ticks (frecuencia * quantum_ms) / 1000
static unsigned int timer_quantum_to_load(unsigned int quantum_ms) {
    unsigned long long ticks;

    ticks = ((unsigned long long)TIMER_INPUT_HZ * quantum_ms) / 1000;

    return (unsigned int)ticks;
}


// Inicializa el timer periódico de QEMU.
void timer_init(void) {
    unsigned int load_value = timer_quantum_to_load(QUANTUM_MS);

    // Cargar el valor del quantum
    TIMER0_LOAD = load_value;

    // Enable | Periodic | Interrupt enable | 32-bit 
    TIMER0_CONTROL = (1 << 7) | (1 << 6) | (1 << 5) | (1 << 1);

    // Habilitar IRQ del timer en el VIC 
    VIC_INTENABLE = TIMER0_IRQ_BIT;
}