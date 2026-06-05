/* os.h
 *
 * Agrupa direcciones de hardware,constantes de syscalls, 
 * codigos de error, modos ARM, estados de proceso y
 * la estructura PCB usada por IRQ, SVC y aborts.
 */

#ifndef OS_BEAGLE_H
#define OS_BEAGLE_H

// UART0 - BeagleBone Black
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

// Frecuencia 24MHz
// Quantum del Round-Robin en milisegundos.
#define TIMER_INPUT_HZ  24000000
#define QUANTUM_MS      100

// INTCPS - Controlador de interrupciones
#define INTCPS_BASE     0x48200000
#define INTC_MIR_CLR2   (INTCPS_BASE + 0xC8)
#define INTC_CONTROL    (INTCPS_BASE + 0x48)
#define TIMER_IRQ_BIT   (1u << 4)

// Clock Manager - Registro para encender el reloj del Timer2.
#define CM_PER_TIMER2   0x44E00080

// Watchdog WDT1 - Se apaga en la Beagle para que no se reinicie.
#define WDT_WSPR        0x44E35048
#define WDT_WWPS        0x44E35034
#define WDT_WWPS_PEND   (1 << 4)

// Direcciones fijas de P1 y P2.
// U-Boot/CoolTerm carga los binarios de usuario en estas direcciones:
//   P1 -> 0x82100000
//   P2 -> 0x82200000
// El kernel construye el PC inicial con estos valores.
#define P1_ENTRY        0x82100000
#define P1_STACK_TOP    0x82112000
#define P2_ENTRY        0x82200000
#define P2_STACK_TOP    0x82212000

// Rangos simples permitidos para validar punteros de usuario.
// Incluyen codigo, rodata, data, bss y el stack que usamos en esta fase.
#define P1_USER_START   0x82100000
#define P1_USER_END     0x82120000
#define P2_USER_START   0x82200000
#define P2_USER_END     0x82220000

// ABI de syscalls Phase 2.
// Usuario coloca el ID en r0 y ejecuta svc #0.
// root.s guarda el contexto y syscall.c interpreta estos IDs.
#define SYS_YIELD       0
#define SYS_EXIT        1
#define SYS_WRITE       2

#define SYSERR_BAD_ID   (-1)
#define SYSERR_BAD_ARG  (-2)
#define SYSERR_BAD_PTR  (-3)
#define SYS_WRITE_MAX   256

// Tipos de fault usados por root.s y fault.c.
#define FAULT_NONE              0
#define FAULT_DATA_ABORT        1
#define FAULT_PREFETCH_ABORT    2
#define FAULT_UNDEFINED_INSTRUCTION 3

// Razones de terminacion guardadas en el PCB.
#define TERM_NONE               0
#define TERM_EXIT               1
#define TERM_FAULT_DATA_ABORT   2
#define TERM_FAULT_PREFETCH_ABORT 3
#define TERM_FAULT_UNKNOWN      4
#define TERM_FAULT_UNDEFINED_INSTRUCTION 5

// Modos ARM
#define CPSR_MODE_USR   0x10
#define CPSR_MODE_IRQ   0x12
#define CPSR_MODE_SYS   0x1F

#define CPSR_IRQ_DISABLE 0x80
#define CPSR_FIQ_DISABLE 0x40

// Usuario en ARM state, IRQ habilitadas, FIQ deshabilitadas.
#define USER_CPSR       (CPSR_MODE_USR | CPSR_FIQ_DISABLE)

// Kernel en SYS mode, IRQ/FIQ deshabilitadas.
#define KERNEL_CPSR     (CPSR_MODE_SYS | CPSR_IRQ_DISABLE | CPSR_FIQ_DISABLE)

// Estructura PCB - Cada proceso tiene un PCB con su contexto guardado.
#define NUM_PROCESSES 3

typedef enum {
    READY = 0,
    RUNNING = 1,
    TERMINATED = 2
} ProcessState;

typedef struct {
    unsigned int pid;        // Identificador del proceso
    unsigned int regs[13];   // Espacio logico para R0-R12; el frame real queda en stack
    unsigned int sp;         // SP del trap-frame guardado para restaurar el proceso
    unsigned int lr;         // Link Register
    unsigned int pc;         // PC
    unsigned int cpsr;       // CPSR esperado al volver a USR
    ProcessState state;      // Estado del proceso

    // Campos requeridos para Phase 2.
    int exit_code;           // Codigo recibido por SYS_EXIT
    int syscall_id;          // Ultimo ID de syscall observado para trazas/debug
    int syscall_ret;         // Ultimo valor de retorno entregado en r0
    int fault_type;          // data_abort, prefetch_abort o none
    int termination_reason;  // TERM_EXIT o razon de fault
    unsigned int user_start; // Inicio del rango valido de memoria de usuario
    unsigned int user_end;   // Fin exclusivo del rango valido de memoria de usuario
} PCB;

// PCB globales del sistema
extern PCB pcb[NUM_PROCESSES];
extern int current_process;

// UART
void uart_putc(char c);
char uart_getc(void);
void uart_putnum(unsigned int num);

// Servicios de input/output
void os_write(const char *s, unsigned int len);
void os_puts(const char *s);
void os_read_line(char *buffer, int max_length);

// Hardware
void disable_watchdog(void);
void timer_init(void);

// PCB / Scheduler
void init_pcbs(void);
int scheduler_pick_next(void);
unsigned int timer_irq_handler(unsigned int current_sp);
unsigned int svc_handler_c(unsigned int current_sp);
unsigned int fault_handler_c(unsigned int current_sp, unsigned int fault_type, unsigned int status, unsigned int fault_addr);

void PUT32(unsigned int addr, unsigned int value);
unsigned int GET32(unsigned int addr);
void enable_irq(void);
void start_first_process(unsigned int sp);

#endif
