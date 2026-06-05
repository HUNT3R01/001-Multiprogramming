#include "user_syscalls.h"

static void long_loop(void);
void p1_main(void);

__attribute__((section(".text.entry"), used))
void p1_start(void) {
    p1_main();
    while (1) {
    }
}

void p1_main(void) {
    static const char start[] = "[TEST timer] P1 timer test start\r\n";
    static const char end[] = "[TEST timer] P1 timer test end\r\n";
    (void)sys_write(1, start, sizeof(start) - 1);
    long_loop();
    (void)sys_write(1, end, sizeof(end) - 1);
    sys_exit(0);
}

static void long_loop(void) {
    volatile unsigned int i;
    for (i = 0; i < 300000000; i++) {
        asm volatile("" ::: "memory");
    }
}
