/* user/P1/main.c
 *
 * Proceso de P1 demuestra:
 *  - SYS_WRITE,
 *  - ventana larga para que el timer lo interrumpa,
 *  - SYS_YIELD,
 *  - continuidad despues de que P2 falla,
 *  - SYS_EXIT al final.
 */

// Proceso 1 
// Cubre SYS_WRITE, SYS_YIELD, preemption por timer y SYS_EXIT.
#include "user_syscalls.h"

static void delay(void);
static void long_preemption_window(void);
void p1_main(void);

// El kernel carga el binario crudo en 0x82100000 y salta al inicio.
// Esta sección garantiza que lo primero en .text sea una entrada real,
// no una función inline auxiliar de user_syscalls.h.
__attribute__((section(".text.entry"), used))
// Entrada real del binario: los linker scripts colocan .text.entry primero.
void p1_start(void) {
    p1_main();
    while (1) {
    }
}

// Flujo principal de P1. Primero deja una ventana larga 
// para que el timer lo interrumpa, luego hace SYS_WRITE + SYS_YIELD varias veces y 
// finalmente hace SYS_EXIT. Si P2 ya fue aislado por fault, P1 debe seguir corriendo normalmente y 
// salir con SYS_EXIT. Si no hay procesos runnable despues de que P1 sale, el kernel debe entrar al idle documentado.
void p1_main(void) {
    static const char preempt[] = "[P1] timer preemption window\r\n";
    static const char msg[] = "[P1] write + yield\r\n";
    static const char exit_msg[] = "[P1] exit\r\n";
    int i;

    // Primero dejamos una ventana larga sin sys_yield(). El quantum se redujo a 100 ms para que la interrupcion sea visible en la demo final.
    // Así el DMTimer2 debe interrumpir a P1 y el log final muestra:
    // reason=timer_irq / reason=dispatch.
    (void)sys_write(1, preempt, sizeof(preempt) - 1);
    long_preemption_window();

    // Luego hacemos syscalls normales para demostrar SYS_WRITE + SYS_YIELD.
    for (i = 0; i < 8; i++) {
        (void)sys_write(1, msg, sizeof(msg) - 1);
        (void)sys_yield();
        delay();
    }

    // Cuando P2 ya fue aislado por fault, P1 sale. Si no hay procesos runnable,
    // el kernel debe entrar al idle documentado.
    (void)sys_write(1, exit_msg, sizeof(exit_msg) - 1);
    sys_exit(0);
}

static void long_preemption_window(void) {
    volatile unsigned int i;

    for (i = 0; i < 300000000; i++) {
        // Evita que el compilador elimine el loop y mantiene ejecución en USR.
        asm volatile("" ::: "memory");
    }
}

static void delay(void) {
    volatile int i;
    for (i = 0; i < 1000000; i++);
}
