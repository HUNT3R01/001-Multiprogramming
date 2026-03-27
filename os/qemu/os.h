#ifndef OS_QEMU_H
#define OS_QEMU_H

typedef unsigned int uint32_t;

// UART0 
#define UART0_BASE      0x101F1000
#define UART_DR         (*(volatile uint32_t *)(UART0_BASE + 0x00))
#define UART_FR         (*(volatile uint32_t *)(UART0_BASE + 0x18))
#define UART_FR_TXFF    (1 << 5)
#define UART_FR_RXFE    (1 << 4)

// Timer0 
#define TIMER0_BASE     0x101E2000
#define TIMER0_LOAD     (*(volatile uint32_t *)(TIMER0_BASE + 0x00))
#define TIMER0_VALUE    (*(volatile uint32_t *)(TIMER0_BASE + 0x04))
#define TIMER0_CONTROL  (*(volatile uint32_t *)(TIMER0_BASE + 0x08))
#define TIMER0_INTCLR   (*(volatile uint32_t *)(TIMER0_BASE + 0x0C))

//   Frecuencia 1MHz
//   Quantum del Round-Robin en milisegundos.
#define TIMER_INPUT_HZ  1000000
#define QUANTUM_MS      10000

// VIC
#define VIC_BASE        0x10140000
#define VIC_INTENABLE   (*(volatile uint32_t *)(VIC_BASE + 0x10))
#define TIMER0_IRQ_BIT  (1 << 4)

//PCB
#define NUM_PROCESSES 3

typedef enum {
    READY = 0,
    RUNNING = 1
} ProcessState;

typedef struct {
    unsigned int pid;
    unsigned int regs[13];
    unsigned int sp;
    unsigned int lr;
    unsigned int pc;
    unsigned int cpsr;
    ProcessState state;
} PCB;

extern PCB pcb[NUM_PROCESSES];
extern int current_process;

extern void p1_main(void);
extern void p2_main(void);

// UART 
void uart_putc(char c);
char uart_getc(void);
void uart_puts(const char *s);
void uart_putnum(unsigned int num);

// IO del OS 
void os_write(const char *s, int len);
void os_puts(const char *s);
void os_read_line(char *buffer, int max_len);

//Hardware 
void timer_init(void);

// PCB / scheduler 
void init_pcbs(void);
unsigned int timer_irq_handler(unsigned int current_sp);

// ASM
void enable_irq(void);
void start_first_process(unsigned int sp);

#endif