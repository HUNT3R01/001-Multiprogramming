.section .text
.syntax unified
.code 32
.globl _start

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
    msr cpsr_c, #0xDF
    ldr sp, =_stack_top

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

    ldr r0, =vector_table
    mcr p15, 0, r0, c12, c0, 0
    dsb
    isb

    bl  main

hang:
    b   hang

irq_handler:
    sub  lr, lr, #4
    stmfd sp!, {r0-r12, lr}
    bl   timer_irq_handler
    ldmfd sp!, {r0-r12, pc}^

.globl PUT32
PUT32:
    str r1, [r0]
    bx  lr

.globl GET32
GET32:
    ldr r0, [r0]
    bx  lr

.globl enable_irq
enable_irq:
    mrs r0, cpsr
    bic r0, r0, #(1 << 7)
    msr cpsr_c, r0
    bx  lr

.section .bss
.align 4
__bss_start__:
_stack_bottom:
    .skip 0x2000
_stack_top:
__bss_end__: