.section .text
.global _start
.global irq_handler_asm 

_start:
ldr r0, =vector_table
mcr p15, 0, r0, c12, c0, 0

ldr r0, =__bss_start__     
ldr r1, =__bss_end__
mov r2, #0

clear_bss_loop:
cmp r0, r1
bge bss_cleared   
str r2, [r0], #4         
b clear_bss_loop

bss_cleared:
dsb                        
isb

ldr sp, =0x82010000
bl main

loop:
b loop

.align 5                        @ La tabla debe estar alineada en memoria
vector_table:
    ldr pc, =_start             @ Reset
    ldr pc, =loop               @ Undefined Instruction
    ldr pc, =loop               @ Software Interrupt (SVC)
    ldr pc, =loop               @ Prefetch Abort
    ldr pc, =loop               @ Data Abort
    ldr pc, =loop               @ Reserved
    ldr pc, =irq_handler_asm    @ IRQ (Interrupciones normales, aquí viene el Timer)
    ldr pc, =loop               @ FIQ (Interrupciones rápidas)

irq_handler_asm:
sub lr, lr, #4              
stmfd sp!, {r0-r12, lr}     
bl timer_irq_handler        
ldmfd sp!, {r0-r12, pc}^