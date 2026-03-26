#ifndef OS_QEMU_H
#define OS_QEMU_H

/* Como el Makefile usa -nostdinc, definimos nuestros propios tipos */
typedef unsigned int uint32_t;

/* --- Direcciones Base de Hardware (Máquina VersatilePB en QEMU) --- */
#define UART0_BASE 0x101F1000
#define TIMER0_BASE 0x101E2000
#define VIC_BASE    0x10140000

/* Macros para acceder a los registros del Timer 0 */
#define TIMER0_LOAD    (*(volatile uint32_t *)(TIMER0_BASE + 0x00))
#define TIMER0_VALUE   (*(volatile uint32_t *)(TIMER0_BASE + 0x04))
#define TIMER0_CONTROL (*(volatile uint32_t *)(TIMER0_BASE + 0x08))
#define TIMER0_INTCLR  (*(volatile uint32_t *)(TIMER0_BASE + 0x0C))

/* Macros para el Vectored Interrupt Controller (VIC) */
#define VIC_INTENABLE  (*(volatile uint32_t *)(VIC_BASE + 0x10))

/* --- Estructuras para Procesos (PCB) --- */
typedef struct {
    uint32_t r[13];     // Registros R0 a R12
    uint32_t sp;        // R13 (Stack Pointer)
    uint32_t lr;        // R14 (Link Register / Return Address)
    uint32_t pc;        // R15 (Program Counter)
    uint32_t cpsr;      // Current Program Status Register
} cpu_context_t;

typedef enum {
    PROC_STATE_READY,
    PROC_STATE_RUNNING
} proc_state_t;

typedef struct {
    int pid;                  // Process ID (0=OS, 1=P1, 2=P2)
    proc_state_t state;       // Estado del proceso
    cpu_context_t context;    // Contexto guardado (registros)
    uint32_t *stack_base;     // Inicio de su memoria RAM (Stack)
} pcb_t;

/* --- Declaraciones de funciones --- */
void uart_putc(char c);
void uart_puts(const char *s);
char uart_getc(void);


/* Mapeo de funciones que pide stdio.c a nuestras funciones UART */
static inline void os_write(const char *buf, int len) {
    for (int i = 0; i < len; i++) {
        uart_putc(buf[i]);
    }
}

static inline void os_read_line(char *buf, int max_len) {
    if (max_len > 0) buf[0] = '\0';
}

#endif