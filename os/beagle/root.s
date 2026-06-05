// root.s
// Todas las rutas guardan el contexto en un trap-frame compatible:
//   r0-r3, r4-r12, lr_usr, cpsr_usr, pc_usr
// Asi el mismo tipo de frame puede restaurarse despues de IRQ, SVC o fault.

.section .text
.syntax unified
.arm
.globl _start
.globl PUT32
.globl GET32
.globl enable_irq
.globl start_first_process

.extern main
.extern timer_irq_handler
.extern svc_handler_c
.extern fault_handler_c
.extern __bss_start__
.extern __bss_end__
.extern _stack_top

// CPSR mode values used in this file.
.equ MODE_IRQ_I_F, 0xD2     // IRQ mode, ARM state, IRQ/FIQ disabled
.equ MODE_SVC_I_F, 0xD3     // SVC mode, ARM state, IRQ/FIQ disabled
.equ MODE_ABT_I_F, 0xD7     // Abort mode, ARM state, IRQ/FIQ disabled
.equ MODE_UND_I_F, 0xDB     // Undefined mode, ARM state, IRQ/FIQ disabled
.equ MODE_SYS_I_F, 0xDF     // SYS mode, ARM state, IRQ/FIQ disabled

.align 5
_start:
vector_table:
    b reset_handler
    b undefined_instruction_handler
    b svc_handler
    b prefetch_abort_handler
    b data_abort_handler
    b .
    b irq_handler
    b hang

reset_handler:
    // Configura stack para modo IRQ.
    msr cpsr_c, #MODE_IRQ_I_F
    ldr sp, =_stack_top

    // Configura stack para modo Abort.
    msr cpsr_c, #MODE_ABT_I_F
    ldr sp, =_stack_top
    sub sp, sp, #0x400

    msr cpsr_c, #MODE_UND_I_F
    ldr sp, =_stack_top
    sub sp, sp, #0x600

    // Configura stack para modo SVC.
    msr cpsr_c, #MODE_SVC_I_F
    ldr sp, =_stack_top
    sub sp, sp, #0x800

    // Configura stack para modo SYS/kernel.
    msr cpsr_c, #MODE_SYS_I_F
    ldr sp, =_stack_top
    sub sp, sp, #0x1000

    // Instala la tabla de vectores.
    ldr r0, =vector_table
    mcr p15, 0, r0, c12, c0, 0

    // Limpia la sección .bss.
    ldr r0, =__bss_start__
    ldr r1, =__bss_end__
    mov r2, #0

clear_bss:
    cmp r0, r1
    bge bss_done
    str r2, [r0], #4
    b clear_bss

bss_done:
    bl main

hang:
    b hang


undefined_instruction_handler:
    sub lr, lr, #4
    stmfd sp!, {r0-r3}

    mov r1, lr
    mrs r2, spsr

    msr cpsr_c, #MODE_SYS_I_F

    stmfd sp!, {r1}
    stmfd sp!, {r2}

    stmfd sp!, {r4-r12, lr}

    msr cpsr_c, #MODE_UND_I_F
    ldmfd sp!, {r0-r3}
    msr cpsr_c, #MODE_SYS_I_F
    stmfd sp!, {r0-r3}

    mov r0, sp
    mov r1, #3
    mov r2, #0
    mov r3, #0
    bl fault_handler_c

    mov sp, r0

    ldmfd sp!, {r0-r3}
    ldmfd sp!, {r4-r12, lr}
    ldmfd sp!, {r2}
    ldmfd sp!, {r1}

    msr cpsr_c, #MODE_UND_I_F
    msr spsr_cxsf, r2
    movs pc, r1

svc_handler:
    // En SVC, LR_svc apunta a la instruccion siguiente al svc #0.
    // No se resta 4 como en IRQ.

    // Guarda R0-R3 temporalmente en stack SVC.
    stmfd sp!, {r0-r3}

    // r1 = PC de retorno a usuario, r2 = CPSR original del usuario.
    mov r1, lr
    mrs r2, spsr

    // Cambia a SYS para acceder al SP/LR compartidos con USR.
    msr cpsr_c, #MODE_SYS_I_F

    // Guarda PC y CPSR del proceso que hizo syscall.
    stmfd sp!, {r1}
    stmfd sp!, {r2}

    // Guarda R4-R12 y LR_usr.
    stmfd sp!, {r4-r12, lr}

    // Recupera R0-R3 desde stack SVC y los guarda en stack del proceso.
    msr cpsr_c, #MODE_SVC_I_F
    ldmfd sp!, {r0-r3}
    msr cpsr_c, #MODE_SYS_I_F
    stmfd sp!, {r0-r3}

    // Llama al dispatcher C de syscalls pasando el SP del trap-frame actual.
    mov r0, sp
    bl svc_handler_c

    // r0 regresa con el SP del proceso a restaurar.
    mov sp, r0

    // Restaura el contexto del proceso seleccionado.
    ldmfd sp!, {r0-r3}
    ldmfd sp!, {r4-r12, lr}
    ldmfd sp!, {r2}       // CPSR destino
    ldmfd sp!, {r1}       // PC destino

    // Retorno estilo excepcion: SPSR_svc -> CPSR y PC = r1.
    msr cpsr_c, #MODE_SVC_I_F
    msr spsr_cxsf, r2
    movs pc, r1


// Handler de prefetch abort.
// Si una instruccion de usuario intenta ejecutar desde una direccion invalida,
// se guarda el contexto, se marca el proceso como terminado y se agenda otro.
prefetch_abort_handler:
    // En prefetch abort, LR_abt apunta a la instruccion faulting + 4.
    sub lr, lr, #4

    // Guarda R0-R3 temporalmente en stack Abort.
    stmfd sp!, {r0-r3}

    // r1 = PC faulting aproximado, r2 = CPSR original del usuario.
    mov r1, lr
    mrs r2, spsr

    // Cambia a SYS para acceder al SP/LR compartidos con USR.
    msr cpsr_c, #MODE_SYS_I_F

    // Guarda PC y CPSR del proceso faulting.
    stmfd sp!, {r1}
    stmfd sp!, {r2}

    // Guarda R4-R12 y LR_usr.
    stmfd sp!, {r4-r12, lr}

    // Recupera R0-R3 desde stack Abort y los guarda en stack del proceso.
    msr cpsr_c, #MODE_ABT_I_F
    ldmfd sp!, {r0-r3}
    msr cpsr_c, #MODE_SYS_I_F
    stmfd sp!, {r0-r3}

    // r0 = trap frame, r1 = tipo fault, r2 = IFSR, r3 = IFAR.
    mov r0, sp
    mov r1, #2
    mrc p15, 0, r2, c5, c0, 1
    mrc p15, 0, r3, c6, c0, 2
    bl fault_handler_c

    // r0 regresa con el SP del proceso sano a restaurar.
    mov sp, r0

    // Restaura el contexto del proceso seleccionado.
    ldmfd sp!, {r0-r3}
    ldmfd sp!, {r4-r12, lr}
    ldmfd sp!, {r2}       // CPSR destino
    ldmfd sp!, {r1}       // PC destino

    // Retorno estilo excepcion: SPSR_abt -> CPSR y PC = r1.
    msr cpsr_c, #MODE_ABT_I_F
    msr spsr_cxsf, r2
    movs pc, r1

// Handler de data abort.
// Si una instruccion de usuario toca una direccion invalida, el proceso se aisla.
data_abort_handler:
    // En data abort, LR_abt apunta a la instruccion faulting + 8.
    sub lr, lr, #8

    // Guarda R0-R3 temporalmente en stack Abort.
    stmfd sp!, {r0-r3}

    // r1 = PC faulting aproximado, r2 = CPSR original del usuario.
    mov r1, lr
    mrs r2, spsr

    // Cambia a SYS para acceder al SP/LR compartidos con USR.
    msr cpsr_c, #MODE_SYS_I_F

    // Guarda PC y CPSR del proceso faulting.
    stmfd sp!, {r1}
    stmfd sp!, {r2}

    // Guarda R4-R12 y LR_usr.
    stmfd sp!, {r4-r12, lr}

    // Recupera R0-R3 desde stack Abort y los guarda en stack del proceso.
    msr cpsr_c, #MODE_ABT_I_F
    ldmfd sp!, {r0-r3}
    msr cpsr_c, #MODE_SYS_I_F
    stmfd sp!, {r0-r3}

    // r0 = trap frame, r1 = tipo fault, r2 = DFSR, r3 = DFAR.
    mov r0, sp
    mov r1, #1
    mrc p15, 0, r2, c5, c0, 0
    mrc p15, 0, r3, c6, c0, 0
    bl fault_handler_c

    // r0 regresa con el SP del proceso sano a restaurar.
    mov sp, r0

    // Restaura el contexto del proceso seleccionado.
    ldmfd sp!, {r0-r3}
    ldmfd sp!, {r4-r12, lr}
    ldmfd sp!, {r2}       // CPSR destino
    ldmfd sp!, {r1}       // PC destino

    // Retorno estilo excepcion: SPSR_abt -> CPSR y PC = r1.
    msr cpsr_c, #MODE_ABT_I_F
    msr spsr_cxsf, r2
    movs pc, r1

irq_handler:
    // En IRQ, LR_irq apunta 4 bytes adelante del retorno real.
    sub lr, lr, #4

    // Guarda R0-R3 temporalmente en stack IRQ.
    stmfd sp!, {r0-r3}

    // r1 = PC de retorno a usuario, r2 = CPSR original del usuario.
    mov r1, lr
    mrs r2, spsr

    // Cambia a SYS para acceder al SP/LR compartidos con USR.
    msr cpsr_c, #MODE_SYS_I_F

    // Guarda PC y CPSR del proceso interrumpido.
    stmfd sp!, {r1}
    stmfd sp!, {r2}

    // Guarda R4-R12 y LR_usr.
    stmfd sp!, {r4-r12, lr}

    // Recupera R0-R3 desde stack IRQ y los guarda en stack del proceso.
    msr cpsr_c, #MODE_IRQ_I_F
    ldmfd sp!, {r0-r3}
    msr cpsr_c, #MODE_SYS_I_F
    stmfd sp!, {r0-r3}

    // Llama al scheduler en C pasando el SP del trap-frame actual.
    mov r0, sp
    bl timer_irq_handler

    // r0 regresa con el SP del siguiente proceso.
    mov sp, r0

    // Restaura el contexto del proceso seleccionado.
    ldmfd sp!, {r0-r3}
    ldmfd sp!, {r4-r12, lr}
    ldmfd sp!, {r2}       // CPSR destino
    ldmfd sp!, {r1}       // PC destino

    // Retorno estilo excepción: SPSR_irq -> CPSR y PC = r1.
    msr cpsr_c, #MODE_IRQ_I_F
    msr spsr_cxsf, r2
    movs pc, r1

// Función usada por C para escribir en memoria mapeada.
PUT32:
    str r1, [r0]
    bx lr

// Función usada por C para leer de memoria mapeada.
GET32:
    ldr r0, [r0]
    bx lr

// Habilita IRQ en CPSR del modo actual.
enable_irq:
    mrs r0, cpsr
    bic r0, r0, #(1 << 7)
    msr cpsr_c, r0
    bx lr

// Restaura el contexto inicial del primer proceso y lo arranca en USR mode.
// r0 = SP del contexto inicial del proceso.
start_first_process:
    // SYS comparte SP/LR con USR, pero permite ejecutar instrucciones privilegiadas.
    msr cpsr_c, #MODE_SYS_I_F

    // sp apunta al frame inicial construido en pcb.c.
    mov sp, r0

    // Restaurar registros visibles del proceso.
    ldmfd sp!, {r0-r3}
    ldmfd sp!, {r4-r12, lr}
    ldmfd sp!, {r2}       // CPSR destino: USER_CPSR
    ldmfd sp!, {r1}       // PC destino: entrada del proceso

    // Usamos SPSR de IRQ para hacer un retorno estilo excepción a USR.
    msr cpsr_c, #MODE_IRQ_I_F
    msr spsr_cxsf, r2
    movs pc, r1

.section .bss
.align 4
_stack_bottom:
    .skip 0x2000
_stack_top:
