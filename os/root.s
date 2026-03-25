.section .text
.syntax unified
.code 32
.globl _start

// Variables puente definidas en os.c

// ============================================================
// Tabla de vectores ARM
// Debe estar al inicio del binario (0x82000000)
// ============================================================
.align 5
_start:
vector_table:
    b reset_handler        // 0x00 Reset
    b hang                 // 0x04 Undefined Instruction
    b hang                 // 0x08 SVC
    b hang                 // 0x0C Prefetch Abort
    b hang                 // 0x10 Data Abort
    b .                    // 0x14 Reserved
    b irq_handler          // 0x18 IRQ — DMTimer2
    b hang                 // 0x1C FIQ

// ============================================================
// Reset handler
// ============================================================
reset_handler:
    // Poner CPU en System mode, IRQs deshabilitadas
    // 0xDF = 11011111 = SYS mode (11111) + I=1 + F=1
    msr cpsr_c, #0xDF

    // Stack del OS
    ldr sp, =_stack_top

    // Limpiar .bss — variables globales empiezan en 0
    ldr r0, =__bss_start__
    ldr r1, =__bss_end__
    mov r2, #0
clear_bss:
    cmp  r0, r1
    bge  bss_done
    str  r2, [r0], #4
    b    clear_bss
bss_done:
    dsb
    isb

    // Instalar tabla de vectores via VBAR (CP15 c12)
    ldr r0, =vector_table
    mcr p15, 0, r0, c12, c0, 0
    dsb
    isb

    // Saltar a main() en C
    bl  main

hang:
    b   hang

// ============================================================
// IRQ handler — context switch completo
//
// Cuando ARM toma una IRQ:
//   - Entra en modo IRQ
//   - LR_irq = PC_interrumpido + 4
//   - El proceso estaba corriendo en SYS mode
//
// Pasos:
//   1. sub lr, #4        → LR_irq apunta a instruccion correcta
//   2. stmfd             → guarda R0-R12 y LR_irq en stack IRQ
//   3. cambiar a SYS     → para leer SP y LR del proceso
//   4. guardar en saved_sp / saved_lr
//   5. volver a IRQ      → llamar timer_irq_handler() en C
//   6. leer next_sp/lr/pc que C preparó
//   7. cambiar a SYS     → escribir SP y LR del siguiente
//   8. volver a IRQ      → poner next_pc en LR_irq
//   9. ldmfd + movs pc   → restaurar R0-R12 y saltar al siguiente
// ============================================================
irq_handler:
    // Paso 1 — ajustar LR para apuntar a instruccion interrumpida
    sub  lr, lr, #4

    // Paso 2 — guardar R0-R12 y LR_irq en el stack del modo IRQ
    stmfd sp!, {r0-r12, lr}

    // Paso 3 — cambiar a SYS mode con IRQs deshabilitadas
    // para poder leer SP_sys y LR_sys del proceso interrumpido
    // 0xDF = SYS mode + I=1 + F=1
    mrs  r0, cpsr           // leer CPSR actual (modo IRQ)
    bic  r1, r0, #0x1F      // limpiar bits de modo
    orr  r1, r1, #0xDF      // SYS mode + IRQs off
    msr  cpsr_c, r1         // cambiar a SYS mode

    // Paso 4 — leer SP y LR del proceso interrumpido
    mov  r2, sp             // r2 = SP_sys del proceso actual
    mov  r3, lr             // r3 = LR_sys del proceso actual

    // Volver a modo IRQ
    msr  cpsr_c, r0

    // Guardar en variables globales para que C los lea
    ldr  r4, =saved_sp
    str  r2, [r4]
    ldr  r4, =saved_lr
    str  r3, [r4]

    // Paso 5 — llamar al Round-Robin scheduler en C
    bl   timer_irq_handler

    // Paso 6 — leer SP, LR y PC del siguiente proceso
    // que C preparó en next_sp, next_lr, next_pc
    ldr  r4, =next_sp
    ldr  r2, [r4]
    ldr  r4, =next_lr
    ldr  r3, [r4]
    ldr  r4, =next_pc
    ldr  r5, [r4]

    // Paso 7 — cambiar a SYS mode para escribir SP y LR
    // del siguiente proceso
    mrs  r0, cpsr
    bic  r1, r0, #0x1F
    orr  r1, r1, #0xDF
    msr  cpsr_c, r1

    mov  sp, r2             // SP_sys del siguiente proceso
    mov  lr, r3             // LR_sys del siguiente proceso

    // Volver a modo IRQ
    msr  cpsr_c, r0

    // Paso 8 — poner el PC del siguiente proceso en LR_irq
    // para que movs pc, lr salte ahí
    mov  lr, r5

    // Paso 9 — restaurar R0-R12 y retornar al siguiente proceso
    // ^ copia SPSR_irq a CPSR — restaura el modo correcto
    ldmfd sp!, {r0-r12}
    movs  pc, lr

// ============================================================
// Acceso a memoria desde C
// ============================================================
.globl PUT32
PUT32:
    str r1, [r0]
    bx  lr

.globl GET32
GET32:
    ldr r0, [r0]
    bx  lr

// ============================================================
// Habilitar IRQ — limpia bit I del CPSR
// ============================================================
.globl enable_irq
enable_irq:
    mrs r0, cpsr
    bic r0, r0, #(1 << 7)
    msr cpsr_c, r0
    bx  lr

// ============================================================
// Stack del OS — 8 KB
// ============================================================
.section .bss
.align 4
__bss_start__:
_stack_bottom:
    .skip 0x2000
_stack_top:
__bss_end__: