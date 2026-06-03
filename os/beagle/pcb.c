/*
 * COMENTARIOS FASE 2 - pcb.c
 *
 * Aqui se construye el contexto inicial de cada proceso de usuario.
 * El objetivo principal es que P1 y P2 empiecen con CPSR en modo USR,
 * no en modo SYS/SVC. Tambien se guardan los rangos de memoria que luego
 * SYS_WRITE usa para validar punteros de usuario antes de leerlos.
 */

#include "os.h"

// Arreglo global de PCBs.
// pcb[0] representa el kernel/idle.
// pcb[1] y pcb[2] son procesos de usuario.
PCB pcb[NUM_PROCESSES];
int current_process = 0;

// Limpia campos de observabilidad usados por syscalls, faults y exit.
static void clear_phase2_fields(PCB *p) {
    p->exit_code = 0;
    p->syscall_id = -1;
    p->syscall_ret = 0;
    p->fault_type = 0;
    p->termination_reason = 0;
}

// Prepara el stack inicial de un proceso de usuario.
//
// El formato del stack debe coincidir con root.s:
//
// low memory:
//   r0-r3
//   r4-r12, lr
//   cpsr
//   pc
// high memory
//
static void setup_stack(PCB *p, unsigned int entry, unsigned int stack_top,
                        unsigned int user_start, unsigned int user_end) {
    // El stack crece hacia abajo, por eso empezamos en stack_top y usamos --sp.
    unsigned int *sp = (unsigned int *)stack_top;
    int i;

    // PC inicial
    // Ultimo dato que root.s sacara del frame: PC inicial del proceso.
    *(--sp) = entry;

    // CPSR inicial: USR mode, ARM state, IRQ habilitadas
    // CPSR de usuario: este valor se cargara en SPSR antes de movs pc, r1.
    *(--sp) = USER_CPSR;

    // LR inicial
    *(--sp) = 0;

    // R12..R0 en cero
    // Registros generales arrancan en cero para tener un contexto conocido.
    for (i = 0; i < 13; i++) {
        *(--sp) = 0;
    }

    p->sp = (unsigned int)sp;
    p->pc = entry;
    p->lr = 0;
    p->cpsr = USER_CPSR;
    p->state = READY;
    p->user_start = user_start;
    p->user_end = user_end;
    clear_phase2_fields(p);
}

// Inicializa los PCBs del sistema.
void init_pcbs(void) {
    pcb[0].pid = 0;
    pcb[0].sp = 0;
    pcb[0].pc = 0;
    pcb[0].lr = 0;
    pcb[0].cpsr = KERNEL_CPSR;
    pcb[0].state = RUNNING;
    pcb[0].user_start = 0;
    pcb[0].user_end = 0;
    clear_phase2_fields(&pcb[0]);

    pcb[1].pid = 1;
    setup_stack(&pcb[1], P1_ENTRY, P1_STACK_TOP, P1_USER_START, P1_USER_END);

    pcb[2].pid = 2;
    setup_stack(&pcb[2], P2_ENTRY, P2_STACK_TOP, P2_USER_START, P2_USER_END);
}
