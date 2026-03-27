#include "os.h"

// Arreglo global de PCBs. pcb[0] para os, pcb[1] y pcb[2] son P1 y P2.
PCB pcb[NUM_PROCESSES];
int current_process = 0;

// Prepara el stack inicial de un proceso.
static void setup_stack(PCB *p, unsigned int entry, unsigned int stack_top) {
    unsigned int *sp = (unsigned int *)stack_top;
    int i;

    *(--sp) = entry;   // PC inicial
    *(--sp) = 0x5F;    // CPSR modo SYS con IRQ habilitadas
    *(--sp) = 0;       // LR inicial

    for (i = 0; i < 13; i++) {
        *(--sp) = 0;   // R12..R0 en cero 
    }

    p->sp = (unsigned int)sp;
    p->pc = entry;
    p->lr = 0;
    p->cpsr = 0x5F;
    p->state = READY;
}

//Inicializa los PCBs del sistema.

void init_pcbs(void) {
    pcb[0].pid = 0;
    pcb[0].state = RUNNING;

    pcb[1].pid = 1;
    setup_stack(&pcb[1], P1_ENTRY, P1_STACK_TOP);

    pcb[2].pid = 2;
    setup_stack(&pcb[2], P2_ENTRY, P2_STACK_TOP);
}