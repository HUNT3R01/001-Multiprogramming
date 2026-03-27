#include "os.h"

// Handler lógico del scheduler para QEMU.
// Limpia el timer, guarda el SP actual y selecciona
// el siguiente proceso con Round-Robin.
unsigned int timer_irq_handler(unsigned int current_sp) {
    TIMER0_INTCLR = 1;

    pcb[current_process].sp = current_sp;
    pcb[current_process].state = READY;

    if (current_process == 0 || current_process == 2) {
        current_process = 1;
    } else {
        current_process = 2;
    }

    pcb[current_process].state = RUNNING;
    return pcb[current_process].sp;
}