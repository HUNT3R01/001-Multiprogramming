#include "os.h"
#include "stdio.h" 

#define UART_DR    (*(volatile uint32_t *)(UART0_BASE + 0x00)) 
#define UART_FR    (*(volatile uint32_t *)(UART0_BASE + 0x18))

pcb_t processes[3];
volatile int current_pid = 0; 
extern void p1_main(void);
extern void p2_main(void);
uint32_t p1_stack[1024]; 
uint32_t p2_stack[1024];

void uart_putc(char c) {
    while (UART_FR & (1 << 5)); 
    UART_DR = c;
}

void uart_puts(const char *s) {
    while (*s != '\0') {
        uart_putc(*s);
        s++;
    }
}

char uart_getc(void) {
    while (UART_FR & (1 << 4));
    return (char)(UART_DR & 0xFF);
}

/* -- Inicializacion del timer -- */
void timer_init(void){
    TIMER0_LOAD = 1000000;
    TIMER0_CONTROL = (1 << 7) | (1 << 6) | (1 << 5) | (1 << 1);
    VIC_INTENABLE = (1 << 4);
}

/* --- Inicializar los Process Control Blocks (PCBs) --- */
void process_init(void) {
    // 0. El OS (Nosotros)
    processes[0].pid = 0;
    processes[0].state = PROC_STATE_RUNNING;

    // --- 1. PROCESO 1 ---
    uint32_t *sp1 = &p1_stack[1024]; 
    *(--sp1) = (uint32_t)p1_main; // 1. PC (Program Counter)
    *(--sp1) = 0x53;              // 2. CPSR (Modo SVC, interrupciones activas)
    *(--sp1) = 0;                 // 3. LR (Link Register)
    for (int i = 0; i < 13; i++) {
        *(--sp1) = 0;             // 4. Registros R12 hasta R0 inicializados en 0
    }
    processes[1].pid = 1;
    processes[1].state = PROC_STATE_READY;
    processes[1].context.sp = (uint32_t)sp1;

    // --- 2. PROCESO 2 ---
    uint32_t *sp2 = &p2_stack[1024];
    *(--sp2) = (uint32_t)p2_main; // 1. PC
    *(--sp2) = 0x53;              // 2. CPSR
    *(--sp2) = 0;                 // 3. LR
    for (int i = 0; i < 13; i++) {
        *(--sp2) = 0;             // 4. Registros R12 hasta R0
    }
    processes[2].pid = 2;
    processes[2].state = PROC_STATE_READY;
    processes[2].context.sp = (uint32_t)sp2;
}

/* --- Planificador Round-Robin (¡Devuelve un puntero de memoria!) --- */
uint32_t* irq_handler_c(uint32_t *sp) {
    // Limpiar la interrupción
    TIMER0_INTCLR = 1;

    // 1. Guardar el SP del proceso que acaba de ser interrumpido
    processes[current_pid].context.sp = (uint32_t)sp;
    processes[current_pid].state = PROC_STATE_READY;

    // 2. Elegir el siguiente proceso (Alternar 1 -> 2 -> 1 -> 2...)
    if (current_pid == 0 || current_pid == 2) {
        current_pid = 1;
    } else {
        current_pid = 2;
    }

    // 3. Marcar como corriendo
    processes[current_pid].state = PROC_STATE_RUNNING;

    // 4. Devolverle a root.s el SP del nuevo proceso
    return (uint32_t *)processes[current_pid].context.sp;
}

int main(void) {
    char tecla;
    
    uart_puts("\n\r======================================\n\r");
    uart_puts("   001 - Multiprogramming      \n\r");
    uart_puts("======================================\n\r");

    process_init();
    timer_init();
    
    PRINT("-> Prueba de numero: %d\n\r", 2024);
    PRINT("-> Prueba de texto: %s\n\r", "Libreria conectada con exito");
    PRINT("-> Prueba de combinacion: El proceso %d esta %s\n\r", 1, "listo");
    
    uart_puts("\n\rEscribe algo:\n\r> ");

    while (1) {
        tecla = uart_getc(); 
        if (tecla == '\r') {
            uart_puts("\n\r> ");
        } else {
            uart_putc(tecla);
        }
    }
    
    return 0;
}