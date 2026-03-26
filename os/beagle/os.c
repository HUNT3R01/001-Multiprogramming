#include "os.h"

// ============================================================
// Globales del OS
// ============================================================
PCB pcb[3]; // Nos aseguramos que tenga espacio para OS(0), P1(1) y P2(2)
int current_process = 0;

// Eliminamos todas las variables puente "saved_sp", "next_sp", etc.

// ============================================================
// Funciones de UART (Se mantienen igual, son de hardware)
// ============================================================
void uart_putc(char c) {
    while ((GET32(UART_LSR) & UART_LSR_TX) == 0);
    PUT32(UART_THR, (unsigned int)c);
}

char uart_getc(void) {
    while ((GET32(UART_LSR) & 0x01) == 0);
    return (char)(GET32(UART_THR) & 0xFF);
}

void uart_putnum(unsigned int num) {
    char buf[10];
    int i = 0;
    if (num == 0) { uart_putc('0'); return; }
    while (num > 0) { buf[i++] = (num % 10) + '0'; num /= 10; }
    while (i > 0) { uart_putc(buf[--i]); }
}

void os_write(const char *s, size_t len) {
    for (size_t i = 0; i < len; i++) uart_putc(s[i]);
}

void os_puts(const char *s) {
    while (*s) { uart_putc(*s++); }
}

// ============================================================
// INICIALIZACIÓN DE LOS PROCESOS (Estilo QEMU)
// ============================================================
void setup_stack(PCB *p, unsigned int entry, unsigned int stack_top) {
    unsigned int *sp = (unsigned int *)stack_top;

    *(--sp) = entry;          // 1. PC (Program Counter)
    *(--sp) = 0x5F;           // 2. CPSR (Modo SYS de BeagleBone, IRQs habilitadas)
    *(--sp) = 0;              // 3. LR (Link Register)
    for (int i = 0; i < 13; i++) {
        *(--sp) = 0;          // 4. Registros R12 a R0 inicializados en 0
    }

    p->sp = (unsigned int)sp;
    p->state = READY;
}

void init_pcbs(void) {
    // 0. El OS
    pcb[0].pid = 0;
    pcb[0].state = RUNNING;

    // 1. Proceso 1
    pcb[1].pid = 1;
    setup_stack(&pcb[1], P1_ENTRY, P1_STACK_TOP);

    // 2. Proceso 2
    pcb[2].pid = 2;
    setup_stack(&pcb[2], P2_ENTRY, P2_STACK_TOP);
}

// ============================================================
// EL CORAZÓN: Cambio de Contexto
// ============================================================
unsigned int timer_irq_handler(unsigned int current_sp) {
    // 1. Apagar la alarma del Hardware de la BeagleBone
    PUT32(TISR, 0x2);         // Limpiar status del DMTimer2
    PUT32(INTC_CONTROL, 0x1); // Desbloquear INTC para la siguiente IRQ

    // 2. Guardar el SP del proceso interrumpido
    pcb[current_process].sp = current_sp;
    pcb[current_process].state = READY;

    // 3. Elegir el siguiente proceso (Round Robin)
    if (current_process == 0 || current_process == 2) {
        current_process = 1;
    } else {
        current_process = 2;
    }

    // 4. Marcar como corriendo y devolver el nuevo SP a Ensamblador
    pcb[current_process].state = RUNNING;
    return pcb[current_process].sp;
}

// ============================================================
// main
// ============================================================
int main(void) {
    disable_watchdog();

    os_puts("\r\n=== OS Multiprogramming ===\r\n");
    os_puts("Target: BeagleBone Black\r\n");

    init_pcbs();
    os_puts("PCBs y stacks inicializados\r\n");

    timer_init();
    os_puts("Timer listo\r\n");

    os_puts("Habilitando interrupciones...\r\n");
    enable_irq();
    os_puts("Listo - iniciando P1\r\n");

    // Primer salto a P1 — cargamos su SP y saltamos a su entry point
    // Esto arranca P1 por primera vez sin esperar el timer
    __asm__ volatile (
        "mov sp, %0  \n"  // cargar SP de P1
        "mov lr, %1  \n"  // LR = entry point de P1
        "movs pc, lr \n"  // saltar a P1 (movs copia SPSR a CPSR)
        :
        : "r" (pcb[0].sp), "r" (pcb[0].pc)
        : "memory"
    );

    // Nunca llega aqui
    while (1);
    return 0;
}