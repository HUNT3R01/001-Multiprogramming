/*
user_syscalls.h
 * Wrappers de usuario. Este archivo es incluido por P1/P2 y encapsula la
 * instruccion svc #0. Gracias a esto el codigo de usuario no llama funciones
 * privilegiadas directamente; solo pide servicios mediante la ABI r0-r3.
 */

#ifndef USER_SYSCALLS_H
#define USER_SYSCALLS_H

#include "stddef.h"

typedef unsigned int uint32_t;
typedef int int32_t;

#define SYS_YIELD 0
#define SYS_EXIT  1
#define SYS_WRITE 2

// Funcion generica de 3 argumentos. Pone la ABI en registros ARM y ejecuta svc.
static inline int32_t syscall3(uint32_t id, uint32_t a1, uint32_t a2, uint32_t a3) {
    register uint32_t r0 asm("r0") = id;
    register uint32_t r1 asm("r1") = a1;
    register uint32_t r2 asm("r2") = a2;
    register uint32_t r3 asm("r3") = a3;

    // svc #0 transfiere control al vector SVC del kernel.
    asm volatile(
        "svc #0\n"
        : "+r"(r0)
        : "r"(r1), "r"(r2), "r"(r3)
        : "memory"
    );

    return (int32_t)r0;
}

// Cede voluntariamente el CPU al scheduler.
static inline int32_t sys_yield(void) {
    return syscall3(SYS_YIELD, 0, 0, 0);
}

// Termina el proceso actual. No debe retornar si el kernel funciona correctamente.
static inline void sys_exit(int32_t code) {
    (void)syscall3(SYS_EXIT, (uint32_t)code, 0, 0);
    while (1) {
    }
}

// Escribe len bytes desde un buffer de usuario validado por el kernel.
static inline int32_t sys_write(int32_t fd, const void *buf, size_t len) {
    return syscall3(SYS_WRITE, (uint32_t)fd, (uint32_t)buf, (uint32_t)len);
}

#endif
