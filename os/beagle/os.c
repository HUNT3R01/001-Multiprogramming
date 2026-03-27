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

// ============================================================
// Leer teclado (UART)
// ============================================================
void os_read_line(char *buffer, int max_length) {
    int i = 0;
    while (i < max_length - 1) {
        char c = uart_getc(); // Leer un caracter del teclado
        
        // Si presiona Enter (\r o \n), terminamos
        if (c == '\r' || c == '\n') {
            uart_putc('\r');
            uart_putc('\n');
            break;
        }
        
        // Manejo básico de la tecla borrar (Backspace)
        if (c == '\b' || c == 127) {
            if (i > 0) {
                i--;
                uart_putc('\b');
                uart_putc(' ');
                uart_putc('\b');
            }
            continue;
        }
        
        // Guardar la letra y mostrarla en pantalla (Echo)
        buffer[i++] = c;
        uart_putc(c);
    }
    buffer[i] = '\0'; // Terminar el string al estilo C
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
    p->cpsr = 0x6000001F;
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
// INICIALIZACIÓN DE HARDWARE (BeagleBone Black)
// ============================================================

void disable_watchdog(void) {
    // Secuencia mágica de ARM para apagar el reinicio automático
    PUT32(WDT_WSPR, 0x0000AAAA);
    while (GET32(WDT_WWPS) & WDT_WWPS_PEND);
    PUT32(WDT_WSPR, 0x00005555);
    while (GET32(WDT_WWPS) & WDT_WWPS_PEND);
}

void timer_init(void) {
    // 1. Encender el reloj del hardware para el DMTimer2
    PUT32(CM_PER_TIMER2, 0x2);

    // 2. Desenmascarar la interrupcion en el controlador (INTC) - IRQ 68
    PUT32(INTC_MIR_CLR2, TIMER_IRQ_BIT);

    // 3. Configurar el DMTimer2
    PUT32(TCLR, 0x0);            // Detener el timer
    PUT32(TLDR, TIMER_LOAD_VAL); // Valor de recarga automática
    PUT32(TCRR, TIMER_LOAD_VAL); // Valor de inicio actual
    PUT32(TISR, 0x7);            // Limpiar interrupciones viejas atrapadas
    PUT32(TIER, 0x2);            // Habilitar interrupcion por desbordamiento (Overflow)
    PUT32(TCLR, 0x3);            // Iniciar timer (Auto-reload + Start)
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
    //enable_irq();
    os_puts("Listo - iniciando P1\r\n");

    // 1. VITAL: Le avisamos al planificador que P1 es quien tiene el turno
    current_process = 1; 

    // 2. Primer salto a P1 — cargamos su SP y saltamos a su entry point real
    __asm__ volatile (
        "mov sp, %0 \n"              // Cargar el SP falso de P1
        "ldmfd sp!, {r0-r3} \n"      // Sacar los primeros 4 registros falsos
        "ldmfd sp!, {r4-r12, lr} \n" // Sacar los otros 10 registros
        "ldmfd sp!, {r2} \n"         // Sacar el CPSR (0x5F que habilita IRQ)
        "ldmfd sp!, {r1} \n"         // Sacar el PC (P1_ENTRY)
        "msr cpsr_cxsf, r2 \n"       // Aplicar el CPSR (¡Esto habilita las interrupciones de forma segura!)
        "bx r1 \n"              // ¡Saltar a P1!
        :
        : "r" (pcb[1].sp)
        : "memory"
    );

    // Nunca llega aqui
    while (1);
    return 0;
}