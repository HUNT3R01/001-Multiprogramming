#include "os.h"

//Globales
PCB pcb[NUM_PROCESSES];
int current_process = 0;

//UART
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

//Watchdog — deshabilitar WDT1
void disable_watchdog(void) {
    PUT32(WDT_WSPR, 0xAAAA);
    while (GET32(WDT_WWPS) & WDT_WWPS_PEND);
    PUT32(WDT_WSPR, 0x5555);
    while (GET32(WDT_WWPS) & WDT_WWPS_PEND);
}

//Timer DMTimer2
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

//Timer IRQ handler
void timer_irq_handler(void) {
    PUT32(TISR,         0x2);
    PUT32(INTC_CONTROL, 0x1);
    os_puts("Tick\r\n");
}

//PCBs
void init_pcbs(void) {
    pcb[0].pid   = 1;
    pcb[0].pc    = P1_ENTRY;
    pcb[0].sp    = P1_STACK_TOP;
    pcb[0].lr    = P1_ENTRY;
    pcb[0].cpsr  = 0x60000010;
    pcb[0].state = READY;

    pcb[1].pid   = 2;
    pcb[1].pc    = P2_ENTRY;
    pcb[1].sp    = P2_STACK_TOP;
    pcb[1].lr    = P2_ENTRY;
    pcb[1].cpsr  = 0x60000010;
    pcb[1].state = READY;

    __asm__ volatile ("dsb" ::: "memory");
}

int main(void) {
    disable_watchdog();

    os_puts("\r\n=== OS Multiprogramming ===\r\n");
    os_puts("Target: BeagleBone Black\r\n");

    init_pcbs();
    os_puts("PCBs inicializados\r\n");

    timer_init();
    os_puts("Timer listo\r\n");

    os_puts("Habilitando interrupciones...\r\n");
    enable_irq();
    os_puts("Listo - esperando Ticks\r\n");

    while (1);
    return 0;
}