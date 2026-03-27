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
.extern __bss_start__
.extern __bss_end__
.extern _stack_top

.align 5
_start:
vector_table:
    b reset_handler
    b hang
    b hang
    b hang
    b hang
    b .
    b irq_handler
    b hang

reset_handler:
    //C onfigura stack para modo IRQ
    msr cpsr_c, #0xD2
    ldr sp, =_stack_top

    // Configura stack para modo SYS/OS
    msr cpsr_c, #0xDF
    ldr sp, =_stack_top
    sub sp, sp, #0x1000

    // Instala la tabla de vectores
    ldr r0, =vector_table
    mcr p15, 0, r0, c12, c0, 0

    // Limpia la sección .bss
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

irq_handler:
    // Ajusta LR para que apunte a la instrucción correcta de retorno
    sub lr, lr, #4

    // Guarda R0-R3 temporalmente en stack IRQ
    stmfd sp!, {r0-r3}

    // Guarda PC de retorno y SPSR
    mov r1, lr
    mrs r2, spsr

    // Cambia a modo SYS para trabajar con la pila del proceso
    msr cpsr_c, #0xDF

    // Guarda PC y CPSR del proceso interrumpido
    stmfd sp!, {r1}
    stmfd sp!, {r2}

    // Guarda R4-R12 y LR
    stmfd sp!, {r4-r12, lr}

    // Recupera R0-R3 desde stack IRQ y los guarda en stack proceso
    msr cpsr_c, #0xD2
    ldmfd sp!, {r0-r3}
    msr cpsr_c, #0xDF
    stmfd sp!, {r0-r3}

    // Llama al scheduler en C pasando el SP actual
    mov r0, sp
    bl timer_irq_handler

    // r0 regresa con el SP del siguiente proceso
    mov sp, r0

    // Restaura el contexto del nuevo proceso
    ldmfd sp!, {r0-r3}
    ldmfd sp!, {r4-r12, lr}
    ldmfd sp!, {r2}
    ldmfd sp!, {r1}

    // Cambia a modo IRQ para restaurar SPSR y retornar
    msr cpsr_c, #0xD2
    msr spsr_cxsf, r2
    movs pc, r1

// Función usada por C para escribir en memoria mapeada
PUT32:
    str r1, [r0]
    bx lr

// Función usada por C para leer de memoria mapeada
GET32:
    ldr r0, [r0]
    bx lr

// Habilita IRQ en CPSR
enable_irq:
    mrs r0, cpsr
    bic r0, r0, #(1 << 7)
    msr cpsr_c, r0
    bx lr

// Restaura el contexto inicial del primer proceso y lo arranca
start_first_process:
    mov sp, r0
    ldmfd sp!, {r0-r3}
    ldmfd sp!, {r4-r12, lr}
    ldmfd sp!, {r2}
    ldmfd sp!, {r1}
    msr cpsr_cxsf, r2
    bx r1

.section .bss
.align 4
_stack_bottom:
    .skip 0x2000
_stack_top: