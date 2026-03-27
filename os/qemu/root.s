.section .text
.syntax unified
.arm
.global _start
.global enable_irq
.global start_first_process

.extern main
.extern timer_irq_handler

_start:
    ldr pc, reset_addr
    ldr pc, undef_addr
    ldr pc, swi_addr
    ldr pc, prefetch_addr
    ldr pc, abort_addr
    nop
    ldr pc, irq_addr
    ldr pc, fiq_addr

reset_addr:    .word reset
undef_addr:    .word hang
swi_addr:      .word hang
prefetch_addr: .word hang
abort_addr:    .word hang
irq_addr:      .word irq_handler
fiq_addr:      .word hang

reset:
    // Copia la tabla de vectores a 0x00000000
    mov r0, #0x0
    ldr r1, =_start
    mov r2, #15

copy_vectors:
    ldr r3, [r1], #4
    str r3, [r0], #4
    subs r2, r2, #1
    bne copy_vectors

    // Stack para IRQ
    msr cpsr_c, #0xD2
    ldr sp, =0x00030000

    // Stack para SVC/OS
    msr cpsr_c, #0x53
    ldr sp, =0x00020000

    bl main

hang:
    b hang

irq_handler:
    // Ajusta LR para retorno correcto
    sub lr, lr, #4

    // Guarda R0-R3 temporalmente en stack IRQ
    stmfd sp!, {r0-r3}

    // Guarda PC retorno y SPSR
    mov r1, lr
    mrs r2, spsr

    // Cambia a modo SVC para usar stack del proceso
    msr cpsr_c, #0xD3
    stmfd sp!, {r1}
    stmfd sp!, {r2}
    stmfd sp!, {r4-r12, lr}

    // Recupera R0-R3 del stack IRQ y los pasa al stack del proceso
    msr cpsr_c, #0xD2
    ldmfd sp!, {r0-r3}
    msr cpsr_c, #0xD3
    stmfd sp!, {r0-r3}

    // Llama al scheduler
    mov r0, sp
    bl timer_irq_handler

    // Cambia al SP del siguiente proceso
    mov sp, r0

    // Restaura el contexto del proceso elegido
    ldmfd sp!, {r0-r3}
    ldmfd sp!, {r4-r12, lr}
    ldmfd sp!, {r2}
    ldmfd sp!, {r1}

    // Restaura CPSR y vuelve al proceso
    msr spsr_cxsf, r2
    movs pc, r1

enable_irq:
    mrs r0, cpsr
    bic r0, r0, #(1 << 7)
    msr cpsr_c, r0
    bx lr

start_first_process:
    mov sp, r0
    ldmfd sp!, {r0-r3}
    ldmfd sp!, {r4-r12, lr}
    ldmfd sp!, {r2}
    ldmfd sp!, {r1}
    msr cpsr_cxsf, r2
    bx r1