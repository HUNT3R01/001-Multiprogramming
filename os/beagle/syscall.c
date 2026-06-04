/* syscall.c
 *
 * Dispatcher de llamadas al sistema. La entrada real ocurre en root.s cuando
 * un proceso de usuario ejecuta svc #0. root.s guarda un trap-frame en el
 * stack del proceso y llama a svc_handler_c().
 */

#include "os.h"

#define TF_R0      0
#define TF_R1      1
#define TF_R2      2
#define TF_R3      3
#define TF_CPSR    14

// Valida que [addr, addr+len) pertenezca al rango del proceso actual.
// Evita que SYS_WRITE lea memoria del kernel o de otro proceso.
static int valid_user_range(unsigned int addr, unsigned int len) {
    unsigned int end;
    PCB *p;

    if (current_process <= 0 || current_process >= NUM_PROCESSES) {
        return 0;
    }

    // Escribir 0 bytes no toca memoria, por eso se acepta.
    if (len == 0) {
        return 1;
    }

    if (addr == 0) {
        return 0;
    }

    // Detecta overflow: si addr+len vuelve hacia atras, el rango es invalido.
    end = addr + len;
    if (end < addr) {
        return 0;
    }

    p = &pcb[current_process];
    if (addr < p->user_start) {
        return 0;
    }

    if (end > p->user_end) {
        return 0;
    }

    return 1;
}

// Implementacion kernel-side de SYS_WRITE.
// Solo se soporta fd=1, que representa UART/stdout en esta fase.
static int sys_write_impl(unsigned int fd, const char *buf, unsigned int len) {
    if (fd != 1) {
        return SYSERR_BAD_ARG;
    }

    if (len > SYS_WRITE_MAX) {
        return SYSERR_BAD_ARG;
    }

    if (!valid_user_range((unsigned int)buf, len)) {
        return SYSERR_BAD_PTR;
    }

    os_write(buf, len);
    return (int)len;
}

// Usado por SYS_YIELD y SYS_EXIT cuando la syscall debe cambiar de proceso.
static unsigned int schedule_after_syscall(void) {
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
    return pcb[current_process].sp;
}

// Entrada C llamada desde root.s cuando un proceso ejecuta svc #0.
// Recibe el SP del trap-frame guardado y devuelve el SP del proceso a restaurar.
unsigned int svc_handler_c(unsigned int current_sp) {
    // Interpretamos el stack guardado por root.s como arreglo de registros.
    unsigned int *frame = (unsigned int *)current_sp;
    unsigned int id = frame[TF_R0];
    int rc = 0;
    int old_pid = current_process;
    unsigned int next_sp = current_sp;
    unsigned int caller_cpsr = frame[TF_CPSR];

    // El frame actual queda asociado al PCB del proceso que hizo svc.
    pcb[current_process].sp = current_sp;
    pcb[current_process].syscall_id = (int)id;

    os_puts("MODE_SWITCH USER_TO_KERNEL pid=");
    uart_putnum(pcb[old_pid].pid);
    os_puts(" reason=syscall id=");
    uart_putnum(id);
    os_puts("\r\n");

    // Verificación mínima: la syscall debe venir desde USR mode.
    if ((caller_cpsr & 0x1F) != CPSR_MODE_USR) {
        rc = SYSERR_BAD_ARG;
        frame[TF_R0] = (unsigned int)rc;
        pcb[current_process].syscall_ret = rc;
    } else if (id == SYS_WRITE) {
        // r1=fd, r2=buf, r3=len. El retorno se coloca en r0 del frame.
        rc = sys_write_impl(frame[TF_R1], (const char *)frame[TF_R2], frame[TF_R3]);
        frame[TF_R0] = (unsigned int)rc;
        pcb[current_process].syscall_ret = rc;
    } else if (id == SYS_YIELD) {
        // Yield no falla en esta fase: marca runnable y agenda otro proceso.
        rc = 0;
        frame[TF_R0] = (unsigned int)rc;
        pcb[current_process].syscall_ret = rc;
        next_sp = schedule_after_syscall();
    } else if (id == SYS_EXIT) {
        // Exit no debe retornar al proceso que llamo; queda TERMINATED.
        rc = 0;
        pcb[current_process].exit_code = (int)frame[TF_R1];
        pcb[current_process].termination_reason = TERM_EXIT;
        pcb[current_process].state = TERMINATED;
        pcb[current_process].syscall_ret = rc;
        next_sp = schedule_after_syscall();
    } else {
        // ID desconocido: respuesta determinista requerida por la especificacion.
        rc = SYSERR_BAD_ID;
        frame[TF_R0] = (unsigned int)rc;
        pcb[current_process].syscall_ret = rc;
    }

    os_puts("MODE_SWITCH KERNEL_TO_USER pid=");
    uart_putnum(pcb[current_process].pid);
    os_puts(" reason=syscall_return id=");
    uart_putnum(id);
    os_puts(" rc=");
    if (rc < 0) {
        os_puts("-");
        uart_putnum((unsigned int)(-rc));
    } else {
        // ID desconocido: respuesta determinista requerida por la especificacion.
        uart_putnum((unsigned int)rc);
    }
    os_puts("\r\n");

    return next_sp;
}
