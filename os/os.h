#ifndef OS_H
#define OS_H

#include <stdint.h>

/* --- Direcciones Base de Hardware (BeagleBone Black AM335x) --- */
#define WDT1_BASE       0x44E35000
#define UART0_BASE      0x44E09000
#define DMTIMER2_BASE   0x48040000
#define INTC_BASE       0x48200000

/* --- Bloque de Control de Procesos (PCB) --- */
typedef enum { READY, RUNNING } ProcessState;

typedef struct {
    uint32_t pid;       // ID del proceso (0=OS, 1=P1, 2=P2)
    uint32_t sp;        // Puntero de Pila (Stack Pointer)
    uint32_t pc;        // Contador de Programa (Program Counter)
    uint32_t lr;        // Link Register
    uint32_t cpsr;      // Registro de Estado (Status Register)
    uint32_t r[13];     // Registros de propósito general R0-R12
    ProcessState state; // Estado actual
} PCB;

/* --- Prototipos de Funciones del OS --- */
void disable_watchdog(void);
void init_uart0(void);
void init_timer(void);
void init_intc(void);
void init_os(void);
void timer_irq_handler(void);

/* --- Prototipos de Funciones de UART (Usadas por stdio.c) --- */
void uart_putc(char c);
void uart_puts(const char *s);
void uart_itoa(int val, char *buffer);
// Funciones dummy (vacías) requeridas por tu stdio.c actual
void print_float(float val);
void uart_gets_input(char *buffer, int max_len);
int uart_atoi(const char *str);
float uart_atof(const char *str);

#endif