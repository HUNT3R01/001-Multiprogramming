#ifndef OS_BEAGLE_H
#define OS_BEAGLE_H

//UART0 - BeagleBone Black
#define UART0_BASE      0x44E09000
#define UART_THR        (UART0_BASE + 0x00)   
#define UART_LSR        (UART0_BASE + 0x14)   
#define UART_LSR_TX     0x20                  
#define UART_LSR_RX     0x01                 

// DMTimer2
#define DMTIMER2_BASE   0x48040000
#define TISR            (DMTIMER2_BASE + 0x28)   
#define TIER            (DMTIMER2_BASE + 0x2C)   
#define TCLR            (DMTIMER2_BASE + 0x38)  
#define TCRR            (DMTIMER2_BASE + 0x3C)  
#define TLDR            (DMTIMER2_BASE + 0x40)   

//   Frecuencia 24MHz
//   Quantum del Round-Robin en milisegundos.
#define TIMER_INPUT_HZ  24000000
#define QUANTUM_MS      10000

//INTCPS - Controlador de interrupciones

#define INTCPS_BASE     0x48200000
#define INTC_MIR_CLR2   (INTCPS_BASE + 0xC8)
#define INTC_CONTROL    (INTCPS_BASE + 0x48)
#define TIMER_IRQ_BIT   (1u << 4)   

// Clock Manager-Registro para encender el reloj del Timer2.
#define CM_PER_TIMER2   0x44E00080

//Watchdog WDT1 - Se apaga en la beagle para que no se reinicie 
#define WDT_WSPR        0x44E35048
#define WDT_WWPS        0x44E35034
#define WDT_WWPS_PEND   (1 << 4)

// Direcciones fijas de P1 y P2
#define P1_ENTRY        0x82100000
#define P1_STACK_TOP    0x82110000
#define P2_ENTRY        0x82200000
#define P2_STACK_TOP    0x82210000

// Estructura PCB - Cada proceso tiene un PCB con su contexto guardado.
#define NUM_PROCESSES 3

typedef enum {
    READY = 0,
    RUNNING = 1
} ProcessState;

typedef struct {
    unsigned int pid;        // Identificador del proceso
    unsigned int regs[13];   // Espacio para R0-R12 
    unsigned int sp;         // Stack Pointer guardado 
    unsigned int lr;         // Link Register
    unsigned int pc;         // PC 
    unsigned int cpsr;       // Estado del procesador
    ProcessState state;      // Estado del proceso
} PCB;

// PCB globales del sistema
extern PCB pcb[NUM_PROCESSES];
extern int current_process;

//UART
void uart_putc(char c);
char uart_getc(void);
void uart_putnum(unsigned int num);

//Servicios de input output 
void os_write(const char *s, unsigned int len);
void os_puts(const char *s);
void os_read_line(char *buffer, int max_length);

//Hardware
void disable_watchdog(void);
void timer_init(void);

//PCB / Scheduler
void init_pcbs(void);
unsigned int timer_irq_handler(unsigned int current_sp);


void PUT32(unsigned int addr, unsigned int value);
unsigned int GET32(unsigned int addr);
void enable_irq(void);
void start_first_process(unsigned int sp);

#endif