#include "os.h"

// Arreglo global de PCBs para QEMU.
PCB pcb[NUM_PROCESSES];
int current_process = 0;

// Stacks locales de P1 y P2 en QEMU.
static unsigned int p1_stack[1024];
static unsigned int p2_stack[1024];

//Prepara el stack inicial de un proceso.
static void setup_stack(PCB *p, void (*entry)(void), unsigned int *stack_top) {
    unsigned int *sp = stack_top;
    int i;

    *(--sp) = (unsigned int)entry;  // PC 
    *(--sp) = 0x53;                 //CPSR 
    *(--sp) = 0;                    // LR 

    for (i = 0; i < 13; i++) {
        *(--sp) = 0;
    }

    p->sp = (unsigned int)sp;
    p->pc = (unsigned int)entry;
    p->lr = 0;
    p->cpsr = 0x53;
    p->state = READY;
}

// Inicializa los PCBs
void init_pcbs(void) {
    pcb[0].pid = 0;
    pcb[0].state = RUNNING;

    pcb[1].pid = 1;
    setup_stack(&pcb[1], p1_main, &p1_stack[1024]);

    pcb[2].pid = 2;
    setup_stack(&pcb[2], p2_main, &p2_stack[1024]);
}