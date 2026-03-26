.section .text
.global _start

_start:
    @ --- 1. TABLA DE VECTORES ARM ---
    ldr pc, reset_addr
    ldr pc, undef_addr
    ldr pc, swi_addr
    ldr pc, prefetch_addr
    ldr pc, abort_addr
    nop
    ldr pc, irq_addr
    ldr pc, fiq_addr

@ --- 2. EL DICCIONARIO DE DIRECCIONES ---
reset_addr:    .word reset
undef_addr:    .word loop
swi_addr:      .word loop
prefetch_addr: .word loop
abort_addr:    .word loop
irq_addr:      .word irq_handler
fiq_addr:      .word loop

reset:
    @ 3. Copiar la tabla Y el diccionario a la direccion 0x00000000
    mov r0, #0x0
    ldr r1, =_start
    mov r2, #15             @ Ahora copiamos 15 palabras (Instrucciones + Diccionario)
copy_vec:
    ldr r3, [r1], #4
    str r3, [r0], #4
    subs r2, r2, #1
    bne copy_vec

    @ 4. Configurar la pila (Stack) para el modo Interrupciones (IRQ)
    msr cpsr_c, #0xD2       @ Cambiar a modo IRQ 
    ldr sp, =0x00030000     @ Memoria exclusiva para el manejador de IRQ

    @ 5. Configurar pila principal (SVC) y HABILITAR interrupciones (Limpiar I-bit)
    msr cpsr_c, #0x53       @ Cambiar a modo Supervisor (SVC)
    ldr sp, =0x00020000     @ Memoria exclusiva para el OS

    @ 6. Saltar a C
    bl main

loop:
    b loop

@ --- 3. MANEJADOR DE INTERRUPCIONES ---
irq_handler:
    @ 1. Ajustar direccion de retorno
    sub lr, lr, #4
    
    @ 2. Guardar R0-R3 temporalmente en la pila de IRQ para usarlos
    stmfd sp!, {r0-r3}
    
    @ Leer el PC de retorno y el estado de interrupciones
    mov r1, lr
    mrs r2, spsr
    
    @ 3. Cambiar a modo Supervisor (SVC) con interrupciones apagadas (0xD3)
    msr cpsr_c, #0xD3
    
    @ 4. Guardar PC y CPSR
    stmfd sp!, {r1}
    stmfd sp!, {r2}
    
    @ 5. Guardar el resto de los registros (R4 a R12 y LR)
    stmfd sp!, {r4-r12, lr}
    
    @ 6. Recuperar R0-R3 de la pila de IRQ y guardarlos en esta pila
    msr cpsr_c, #0xD2       @ Volver a IRQ temporalmente
    ldmfd sp!, {r0-r3}
    msr cpsr_c, #0xD3       @ Regresar a SVC
    stmfd sp!, {r0-r3}
    
    @ 7. SP actual (del proceso) a C
    mov r0, sp
    bl irq_handler_c
    
    @ 8. SP del nuevo proceso
    mov sp, r0
    
    @ 9. Restaurar registros del nuevo proceso
    ldmfd sp!, {r0-r3}
    ldmfd sp!, {r4-r12, lr}
    ldmfd sp!, {r2}         @ Leer CPSR
    ldmfd sp!, {r1}         @ Leer PC
    
    @ 10. Restaurar CPSR y hacer el salto al proceso
    msr spsr_cxsf, r2
    movs pc, r1

