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
    @ 1. Poner CPU en Modo IRQ (0xD2) y darle su propio Stack arriba de todo
    msr cpsr_c, #0xD2
    ldr sp, =_stack_top

    @ 2. Poner CPU en Modo SYS (0xDF) y darle su Stack al OS (4KB más abajo)
    msr cpsr_c, #0xDF
    ldr sp, =_stack_top
    sub sp, sp, #0x1000

    @ Limpiar .bss — variables globales empiezan en 0
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
// IRQ handler — DMTimer2
// ============================================================
irq_handler:
    @ 1. Ajustar direccion de retorno
    sub lr, lr, #4
    
    @ 2. Guardar R0-R3 temporalmente en la pila de IRQ para usarlos
    stmfd sp!, {r0-r3}
    
    @ Leer el PC de retorno y el estado de interrupciones
    mov r1, lr
    mrs r2, spsr
    
    @ 3. Cambiar a modo System (SYS) con interrupciones apagadas (0xDF)
    msr cpsr_c, #0xDF
    
    @ --- AHORA ESTAMOS EN LA PILA PROPIA DEL PROCESO ---
    @ 4. Guardar PC y CPSR
    stmfd sp!, {r1}
    stmfd sp!, {r2}
    
    @ 5. Guardar el resto de los registros (R4 a R12 y LR)
    stmfd sp!, {r4-r12, lr}
    
    @ 6. Recuperar R0-R3 de la pila de IRQ y guardarlos en esta pila
    msr cpsr_c, #0xD2       @ Volver a IRQ temporalmente
    ldmfd sp!, {r0-r3}
    msr cpsr_c, #0xDF       @ Regresar a SYS
    stmfd sp!, {r0-r3}
    
    @ 7. PASAR EL BALON: Mandar SP a C
    mov r0, sp
    bl timer_irq_handler
    
    @ 8. RECIBIR EL BALON: C nos devuelve el nuevo SP
    mov sp, r0
    
    @ 9. Restaurar registros del nuevo proceso (SYS Mode)
    ldmfd sp!, {r0-r3}
    ldmfd sp!, {r4-r12, lr}
    ldmfd sp!, {r2}         @ Leer CPSR
    ldmfd sp!, {r1}         @ Leer PC
    
    @ 10. Restaurar CPSR y PC volviendo a IRQ
    msr cpsr_c, #0xD2       @ Cambiar a IRQ para el salto final
    msr spsr_cxsf, r2       @ Cargar el CPSR guardado en el SPSR
    movs pc, r1             @ Salto mágico con 's' para aplicar el SPSR

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
_stack_bottom:
    .skip 0x2000
_stack_top:

