/*fault.c
 *
 * Maneja faults que vienen desde modo usuario, principalmente data_abort
 * La politica de esta fase es contener el fallo: el proceso
 * culpable se marca TERMINATED y el kernel sigue con otro proceso runnable.
 *
 * Si el fault viene desde modo privilegiado, se considera bug del kernel y se
 * hace halt porque continuar seria inseguro.
 */

#include "os.h"

#define TF_CPSR    14
#define TF_PC      15

// Convierte el codigo interno a texto para las trazas seriales.
static const char *fault_name(unsigned int fault_type) {
    if (fault_type == FAULT_DATA_ABORT) {
        return "data_abort";
    }

    if (fault_type == FAULT_PREFETCH_ABORT) {
        return "prefetch_abort";
    }

    if (fault_type == FAULT_UNDEFINED_INSTRUCTION) {
        return "undefined_instruction";
    }

    return "unknown";
}

// Traduce el tipo de fault a la razon de terminacion guardada en el PCB.
static int termination_reason_for_fault(unsigned int fault_type) {
    if (fault_type == FAULT_DATA_ABORT) {
        return TERM_FAULT_DATA_ABORT;
    }

    if (fault_type == FAULT_PREFETCH_ABORT) {
        return TERM_FAULT_PREFETCH_ABORT;
    }

    if (fault_type == FAULT_UNDEFINED_INSTRUCTION) {
        return TERM_FAULT_UNDEFINED_INSTRUCTION;
    }

    return TERM_FAULT_UNKNOWN;
}

// Handler C de faults de usuario.
// Recibe el SP del trap-frame, el tipo de fault y registros de diagnostico.
// Devuelve el SP del siguiente proceso runnable que se va a restaurar.
unsigned int fault_handler_c(unsigned int current_sp,
                             unsigned int fault_type,
                             unsigned int status,
                             unsigned int fault_addr) {
    unsigned int *frame = (unsigned int *)current_sp;
    unsigned int caller_cpsr = frame[TF_CPSR];
    unsigned int fault_pc = frame[TF_PC];
    int old_pid = current_process;

    // Guardamos el frame faulting para diagnostico y consistencia del PCB.
    pcb[current_process].sp = current_sp;
    pcb[current_process].fault_type = (int)fault_type;

    os_puts("MODE_SWITCH USER_TO_KERNEL pid=");
    uart_putnum(pcb[old_pid].pid);
    os_puts(" reason=fault type=");
    os_puts(fault_name(fault_type));
    os_puts("\r\n");

    os_puts("FAULT_INFO pid=");
    uart_putnum(pcb[old_pid].pid);
    os_puts(" pc=");
    uart_putnum(fault_pc);
    os_puts(" addr=");
    uart_putnum(fault_addr);
    os_puts(" status=");
    uart_putnum(status);
    os_puts("\r\n");

    // Si el fault no vino desde USR, es un bug del kernel: no intentamos continuar.
    if ((caller_cpsr & 0x1F) != CPSR_MODE_USR) {
        os_puts("KERNEL_FAULT: fault desde modo privilegiado. Halt.\r\n");
        while (1) {
        }
    }

    // aislar el proceso faulting terminandolo.
    pcb[current_process].termination_reason = termination_reason_for_fault(fault_type);
    pcb[current_process].state = TERMINATED;

    // Despues de aislar al proceso culpable, elegimos un proceso sano.
    current_process = scheduler_pick_next();

    if (current_process == 0) {
        os_puts("No hay procesos runnable despues del fault. Kernel idle.\r\n");
        while (1) {
        }
    }

    pcb[current_process].state = RUNNING;

    os_puts("MODE_SWITCH KERNEL_TO_USER pid=");
    uart_putnum(pcb[current_process].pid);
    os_puts(" reason=fault_recovery\r\n");

    return pcb[current_process].sp;
}
