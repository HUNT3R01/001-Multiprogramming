#include "os.h"

//Convierte el quantum en milisegundos al valor de recarga del DMTimer2.
//En Beagle el timer interrumpe al overflow, por eso: load = 2^32 - ticks
//Como TIMER_INPUT_HZ es divisible por 1000, evitamos usar enteros de 64 bits y calculamos primero ticks por ms.
static unsigned int timer_quantum_to_load(unsigned int quantum_ms) {
    unsigned int ticks_per_ms;
    unsigned int ticks;

    // 24 MHz / 1000 = 24000 ticks por ms 
    ticks_per_ms = TIMER_INPUT_HZ / 1000;
    ticks = ticks_per_ms * quantum_ms;

    return (unsigned int)(0 - ticks);
}

// Inicializa DMTimer2 para generar interrupciones periódicas.
void timer_init(void) {
    unsigned int load_value = timer_quantum_to_load(QUANTUM_MS);

    // Encender el reloj del timer 
    PUT32(CM_PER_TIMER2, 0x2);

    // Habilitar la IRQ del timer en el INTC
    PUT32(INTC_MIR_CLR2, TIMER_IRQ_BIT);

    // Detener timer antes de configurarlo 
    PUT32(TCLR, 0x0);

    // Cargar valor inicial y valor de recarga
    PUT32(TLDR, load_value);
    PUT32(TCRR, load_value);

    // Limpiar interrupciones pendientes 
    PUT32(TISR, 0x7);

    // Habilitar interrupción por overflow 
    PUT32(TIER, 0x2);

    // Auto-reload + start 
    PUT32(TCLR, 0x3);
}