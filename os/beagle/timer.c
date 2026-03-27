#include "os.h"

// Inicializa DMTimer2 para generar interrupciones periódicas.
// Este timer es la base del scheduler Round-Robin.
void timer_init(void) {
    // Encender el reloj del timer 
    PUT32(CM_PER_TIMER2, 0x2);

    // Habilitar la IRQ en el controlador de interrupciones
    PUT32(INTC_MIR_CLR2, TIMER_IRQ_BIT);

    // Detener el timer antes de configurarlo
    PUT32(TCLR, 0x0);

    // Cargar valor de recarga y valor inicial
    PUT32(TLDR, TIMER_LOAD_VAL);
    PUT32(TCRR, TIMER_LOAD_VAL);

    // Limpiar flags de interrupción anteriores
    PUT32(TISR, 0x7);

    // Habilitar interrupción por overflow
    PUT32(TIER, 0x2);

    //Arrancar timer en modo auto-reload 
    PUT32(TCLR, 0x3);
}