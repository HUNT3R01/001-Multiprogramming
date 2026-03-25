#include "os.h"

// ============================================================
// Globales del OS
// ============================================================
PCB pcb[NUM_PROCESSES];
int current_process = 0;

// ============================================================
// Variables puente con root.s para el context switch
// root.s las lee y escribe directamente por nombre
// ============================================================
unsigned int saved_sp = 0;  // SP del proceso que fue interrumpido
unsigned int saved_lr = 0;  // LR del proceso que fue interrumpido
unsigned int next_sp  = 0;  // SP del proceso que va a correr
unsigned int next_lr  = 0;  // LR del proceso que va a correr
unsigned int next_pc  = 0;  // PC del proceso que va a correr

// ============================================================
// UART
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
    if (num == 0) { uart_putc('0'); uart_putc('\n'); return; }
    while (num > 0 && i < 10) { buf[i++] = (num % 10) + '0'; num /= 10; }
    while (i > 0) uart_putc(buf[--i]);
    uart_putc('\n');
}

void os_write(const char *s, size_t len) {
    for (size_t i = 0; i < len && s[i]; i++)
        uart_putc(s[i]);
}

void os_puts(const char *s) {
    while (*s) uart_putc(*s++);
}

void os_read_line(char *buffer, int max_length) {
    int i = 0;
    char c;
    while (i < max_length - 1) {
        c = uart_getc();
        if (c == '\n' || c == '\r') { uart_putc('\n'); break; }
        uart_putc(c);
        buffer[i++] = c;
    }
    buffer[i] = '\0';
}

// ============================================================
// Watchdog
// ============================================================
void disable_watchdog(void) {
    PUT32(WDT_WSPR, 0xAAAA);
    while (GET32(WDT_WWPS) & WDT_WWPS_PEND);
    PUT32(WDT_WSPR, 0x5555);
    while (GET32(WDT_WWPS) & WDT_WWPS_PEND);
}

// ============================================================
// Timer DMTimer2
// ============================================================
void timer_init(void) {
    PUT32(CM_PER_TIMER2,  0x2);
    PUT32(INTC_MIR_CLR2,  TIMER_IRQ_BIT);
    PUT32(TCLR,           0x0);
    PUT32(TISR,           0x7);
    PUT32(TLDR,           TIMER_LOAD_VAL);
    PUT32(TCRR,           TIMER_LOAD_VAL);
    PUT32(TIER,           0x2);
    PUT32(TCLR,           0x3);
}

// ============================================================
// Timer IRQ handler — Round-Robin scheduler
//
// Flujo:
//   1. Limpiar la interrupcion del timer e INTC
//   2. Guardar SP y LR del proceso actual (vienen de root.s
//      via saved_sp y saved_lr)
//   3. Elegir el siguiente proceso (Round-Robin)
//   4. Poner SP, LR y PC del siguiente en next_sp/next_lr/next_pc
//      para que root.s los restaure al retornar
// ============================================================
void timer_irq_handler(void) {
    // 1. Limpiar interrupcion
    PUT32(TISR,         0x2);  // limpiar flag del timer
    PUT32(INTC_CONTROL, 0x1);  // ACK al INTC

    // 2. Guardar SP y LR del proceso actual en su PCB
    pcb[current_process].sp    = saved_sp;
    pcb[current_process].lr    = saved_lr;
    pcb[current_process].state = READY;

    // 3. Round-Robin — elegir siguiente proceso
    // Si current=0 (P1) → siguiente=1 (P2)
    // Si current=1 (P2) → siguiente=0 (P1)
    current_process = (current_process + 1) % NUM_PROCESSES;
    pcb[current_process].state = RUNNING;

    // 4. Pasar SP, LR y PC del siguiente a root.s
    next_sp = pcb[current_process].sp;
    next_lr = pcb[current_process].lr;
    next_pc = pcb[current_process].pc;
}

// ============================================================
// setup_stack — sección 3.4 del PDF
//
// Construye un frame de 14 palabras en el stack del proceso:
//   [SP] → R0=0, R1=0, ... R12=0, LR=entry_point
//
// Cuando root.s haga ldmfd sp!, {r0-r12, pc}^
// carga R0-R12 con ceros y salta al entry_point
// ============================================================
static void setup_stack(PCB *p, unsigned int entry, unsigned int stack_top) {
    unsigned int *sp = (unsigned int *)stack_top;

    *(--sp) = entry;  // LR = entry point
    *(--sp) = 0;      // R12
    *(--sp) = 0;      // R11
    *(--sp) = 0;      // R10
    *(--sp) = 0;      // R9
    *(--sp) = 0;      // R8
    *(--sp) = 0;      // R7
    *(--sp) = 0;      // R6
    *(--sp) = 0;      // R5
    *(--sp) = 0;      // R4
    *(--sp) = 0;      // R3
    *(--sp) = 0;      // R2
    *(--sp) = 0;      // R1
    *(--sp) = 0;      // R0  ← SP queda aquí

    p->sp    = (unsigned int)sp;
    p->pc    = entry;
    p->lr    = entry;
    p->cpsr  = 0x60000010;
    p->state = READY;
}

// ============================================================
// init_pcbs
// ============================================================
void init_pcbs(void) {
    pcb[0].pid = 1;
    setup_stack(&pcb[0], P1_ENTRY, P1_STACK_TOP);

    pcb[1].pid = 2;
    setup_stack(&pcb[1], P2_ENTRY, P2_STACK_TOP);

    __asm__ volatile ("dsb" ::: "memory");
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