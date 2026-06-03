/*
 * COMENTARIOS FASE 2 - main.c
 *
 * Punto de entrada del kernel para BeagleBone Black.
 * Este archivo solo hace la inicializacion de alto nivel:
 *  1) apaga el watchdog,
 *  2) inicializa PCBs y stacks de usuario,
 *  3) inicializa el timer de Round-Robin,
 *  4) lanza el primer proceso con un retorno a modo USR.
 *
 * La llamada start_first_process() NO es un salto normal: root.s usa SPSR/CPSR
 * para salir del kernel y entrar al primer proceso con privilegios de usuario.
 */

#include "os.h"

int main(void) {
    // La Beagle reinicia si el watchdog sigue activo; se deshabilita primero.
    disable_watchdog();

    os_puts("\r\n=== OS Multiprogramming - Phase 2 Beagle ===\r\n");
    os_puts("Target: BeagleBone Black\r\n");

    // Construye los PCBs y los trap-frames iniciales de P1/P2 en modo USR.
    init_pcbs();
    os_puts("PCBs y stacks inicializados\r\n");

    // Activa DMTimer2 para generar IRQs de Round-Robin.
    timer_init();
    os_puts("Timer listo\r\n");

    os_puts("Listo - iniciando P1 en modo USR\r\n");

    // P1 es el primer proceso seleccionado para salir del kernel.
    current_process = 1;
    pcb[0].state = READY;
    pcb[1].state = RUNNING;

    os_puts("MODE_SWITCH KERNEL_TO_USER pid=");
    uart_putnum(pcb[current_process].pid);
    os_puts(" reason=initial_launch\r\n");

    // No regresa bajo ejecucion normal: root.s restaura el frame y entra a USR.
    start_first_process(pcb[current_process].sp);

    while (1) {
    }

    return 0;
}
