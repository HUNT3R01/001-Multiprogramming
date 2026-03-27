#include "os.h"

// Handler lógico del scheduler.
unsigned int timer_irq_handler(unsigned int current_sp) {
    // Limpiar la interrupción del timer 
    PUT32(TISR, 0x2);

    // Avisar al INTC que la IRQ ya fue atendida 
    PUT32(INTC_CONTROL, 0x1);

    // Guardar SP del proceso actual 
    pcb[current_process].sp = current_sp;
    pcb[current_process].state = READY;

    // Round-Robin entre P1 y P2
    if (current_process == 0 || current_process == 2) {
        current_process = 1;
    } else {
        current_process = 2;
    }

    pcb[current_process].state = RUNNING;
    return pcb[current_process].sp;
}