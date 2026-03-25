#ifndef OS_H
#define OS_H

/* ============================================================
   BeagleBone Black — AM335x hardware addresses
   ============================================================ */

/* UART0 */
#define UART0_BASE      0x44E09000
#define UART_THR        (UART0_BASE + 0x00)
#define UART_LSR        (UART0_BASE + 0x14)
#define UART_LSR_TX     0x20

/* DMTimer2 */
#define DMTIMER2_BASE   0x48040000
#define TCLR            (DMTIMER2_BASE + 0x38)
#define TCRR            (DMTIMER2_BASE + 0x3C)
#define TLDR            (DMTIMER2_BASE + 0x40)
#define TIER            (DMTIMER2_BASE + 0x2C)
#define TISR            (DMTIMER2_BASE + 0x28)
#define TIMER_LOAD_VAL  0xFE91CA00      /* ~2 seg a 24 MHz     */

/* INTCPS */
#define INTCPS_BASE     0x48200000
#define INTC_MIR_CLR2   (INTCPS_BASE + 0xC8)
#define INTC_CONTROL    (INTCPS_BASE + 0x48)
#define TIMER_IRQ_BIT   (1u << 4)       /* IRQ 68 bit 4        */

/* Clock Manager */
#define CM_PER_TIMER2   0x44E00080

/* Watchdog WDT1 */
#define WDT_WSPR        0x44E35048
#define WDT_WWPS        0x44E35034
#define WDT_WWPS_PEND   (1 << 4)

/* Direcciones de procesos */
#define P1_ENTRY        0x82100000
#define P1_STACK_TOP    0x82110000
#define P2_ENTRY        0x82200000
#define P2_STACK_TOP    0x82210000

/* ============================================================
   Tipos
   ============================================================ */
typedef unsigned int size_t;

/* ============================================================
   PCB — Process Control Block
   ============================================================ */
#define NUM_PROCESSES 2

typedef enum { READY = 0, RUNNING = 1 } ProcessState;

typedef struct {
    unsigned int pid;
    unsigned int regs[13];  /* R0 – R12                    */
    unsigned int sp;
    unsigned int lr;
    unsigned int pc;
    unsigned int cpsr;
    ProcessState state;
} PCB;

extern PCB pcb[NUM_PROCESSES];
extern int current_process;

/* ============================================================
   Prototipos
   ============================================================ */
void uart_putc(char c);
char uart_getc(void);
void uart_putnum(unsigned int num);
void os_write(const char *s, size_t len);
void os_read_line(char *buffer, int max_length);
void os_puts(const char *s);

void disable_watchdog(void);
void timer_init(void);
void timer_irq_handler(void);
void init_pcbs(void);

void PUT32(unsigned int addr, unsigned int value);
unsigned int GET32(unsigned int addr);
void enable_irq(void);

#endif /* OS_H */

// ============================================================
// Variables puente entre root.s y os.c para el context switch
// root.s las escribe/lee directamente por nombre
// ============================================================
extern unsigned int saved_sp;  // SP del proceso interrumpido
extern unsigned int saved_lr;  // LR del proceso interrumpido
extern unsigned int next_sp;   // SP del siguiente proceso
extern unsigned int next_lr;   // LR del siguiente proceso
extern unsigned int next_pc;   // PC del siguiente proceso