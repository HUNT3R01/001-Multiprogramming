/* scheduler.c
 *
 * Scheduler Round-Robin usado por dos rutas:
 *
 * Si solo queda un proceso vivo, el scheduler puede devolver el mismo PID.
 * Eso es normal y por eso en la demo aparece dispatch pid=1 despues de que
 * P2 ya fue terminado por data_abort.
 */

#include "os.h"

// Devuelve el siguiente proceso runnable usando Round-Robin entre P1 y P2.
int scheduler_pick_next(void) {
    int next;

    if (current_process == 1) {
        next = 2;
    } else {
        next = 1;
    }

    // Preferimos el otro proceso si no esta terminado.
    if (pcb[next].state != TERMINATED) {
        return next;
    }

    // Si el otro ya murio, el proceso actual puede seguir corriendo.
    if (current_process != 0 && pcb[current_process].state != TERMINATED) {
        return current_process;
    }

    // 0 significa que no quedan procesos de usuario runnable.
    return 0;
}

// Handler logico del scheduler llamado desde irq_handler en root.s.
unsigned int timer_irq_handler(unsigned int current_sp) {
    int old_pid;

    // Limpiar la interrupción del timer.
    PUT32(TISR, 0x2);

    // Avisar al INTC que la IRQ ya fue atendida.
    PUT32(INTC_CONTROL, 0x1);

    old_pid = current_process;

    os_puts("MODE_SWITCH USER_TO_KERNEL pid=");
    uart_putnum(pcb[old_pid].pid);
    os_puts(" reason=timer_irq\r\n");

    // Guardar SP del proceso actual.
    // current_sp apunta al trap-frame que root.s acaba de guardar.
    pcb[current_process].sp = current_sp;

    if (pcb[current_process].state != TERMINATED) {
        pcb[current_process].state = READY;
    }

    current_process = scheduler_pick_next();

    if (current_process == 0) {
        os_puts("No hay procesos runnable. Kernel idle.\r\n");
        while (1) {
        }
    }

    pcb[current_process].state = RUNNING;

    os_puts("MODE_SWITCH KERNEL_TO_USER pid=");
    uart_putnum(pcb[current_process].pid);
    os_puts(" reason=dispatch\r\n");

    return pcb[current_process].sp;
}
