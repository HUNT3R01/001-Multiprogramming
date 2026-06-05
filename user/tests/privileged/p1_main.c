#include "user_syscalls.h"

static void delay(void) {
    volatile unsigned int i;

    for (i = 0; i < 50000000u; i++) {
        asm volatile("" ::: "memory");
    }
}

void p1_main(void) {
    int i;
    const char start[] = "[TEST privileged] P1 yield to P2\r\n";
    const char alive[] = "[TEST privileged] P1 alive after privileged fault\r\n";
    const char exit_msg[] = "[TEST privileged] P1 exit\r\n";

    sys_write(1, start, sizeof(start) - 1);
    sys_yield();

    for (i = 0; i < 3; i++) {
        sys_write(1, alive, sizeof(alive) - 1);
        sys_yield();
        delay();
    }

    sys_write(1, exit_msg, sizeof(exit_msg) - 1);
    sys_exit(0);

    while (1) {
    }
}

__attribute__((section(".text.entry"), used))
void p1_start(void) {
    p1_main();

    while (1) {
    }
}
