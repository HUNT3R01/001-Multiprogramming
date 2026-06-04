/* user/P2/main.c
 *
 * Proceso de prueba negativa. P2 demuestra:
 *  - SYS_WRITE y SYS_YIELD normales,
 *  - errores controlados rc=-2, rc=-3 y rc=-1,
 *  - data_abort intencional para probar aislamiento de faults.
 */

// Proceso 2
// Cubre pruebas negativas de syscall y fault containment por data_abort.
#include "user_syscalls.h"

static void delay(void);
static void force_data_abort(void);
void p2_main(void);

// El kernel carga el binario crudo en 0x82200000 y salta al inicio.
// Esta sección garantiza que lo primero en .text sea una entrada real,
// no una función inline auxiliar de user_syscalls.h.
__attribute__((section(".text.entry"), used))
// Entrada real del binario: los linker scripts colocan .text.entry primero.
void p2_start(void) {
    p2_main();
    while (1) {
    }
}

// Flujo principal de P2 para validar errores y provocar data_abort.
void p2_main(void) {
    static const char start[] = "[P2] syscall validation\r\n";
    static const char msg[] = "[P2] write + yield\r\n";
    static const char neg_ok[] = "[P2] negative syscall tests OK\r\n";
    static const char neg_fail[] = "[P2] negative syscall tests FAIL\r\n";
    static const char bad[] = "[P2] forcing data abort\r\n";
    int32_t r_bad_fd;
    int32_t r_bad_ptr;
    int32_t r_bad_id;
    int i;

    (void)sys_write(1, start, sizeof(start) - 1);

    for (i = 0; i < 2; i++) {
        (void)sys_write(1, msg, sizeof(msg) - 1);
        (void)sys_yield();
        delay();
    }

    // Pruebas negativas requeridas 
    // - descriptor inválido debe regresar -2
    // - puntero inválido en SYS_WRITE debe regresar -3 sin tumbar el kernel
    // - syscall desconocida debe regresar -1
    r_bad_fd = sys_write(9, msg, sizeof(msg) - 1);
    r_bad_ptr = sys_write(1, (const void *)0xFFFFFFFF, 8);
    r_bad_id = syscall3(99u, 0, 0, 0);

    if (r_bad_fd == -2 && r_bad_ptr == -3 && r_bad_id == -1) {
        (void)sys_write(1, neg_ok, sizeof(neg_ok) - 1);
    } else {
        (void)sys_write(1, neg_fail, sizeof(neg_fail) - 1);
    }

    (void)sys_yield();
    delay();

    // Esta prueba sí provoca una excepción de usuario. El kernel debe terminar P2
    // y continuar ejecutando P1.
    (void)sys_write(1, bad, sizeof(bad) - 1);
    force_data_abort();

    // No debería llegar aquí si el fault handler funciona.
    sys_exit(1);
}

// Acceso intencionalmente invalido: debe entrar al data_abort_handler.
static void force_data_abort(void) {
    volatile unsigned int *bad_ptr = (volatile unsigned int *)0xFFFFFFFF;
    volatile unsigned int value;

    value = *bad_ptr;
    (void)value;
}

static void delay(void) {
    volatile int i;
    for (i = 0; i < 1000000; i++);
}
