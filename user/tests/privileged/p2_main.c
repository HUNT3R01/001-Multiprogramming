#include "user_syscalls.h"

static void force_privileged_instruction(void) {
    unsigned int value;

    asm volatile("mrc p15, 0, %0, c1, c0, 0" : "=r"(value) : : "memory");
    (void)value;
}

void p2_main(void) {
    const char start[] = "[TEST privileged] P2 executing CP15 instruction\r\n";
    const char bad[] = "[TEST privileged] ERROR: P2 survived\r\n";

    sys_write(1, start, sizeof(start) - 1);
    force_privileged_instruction();

    sys_write(1, bad, sizeof(bad) - 1);
    sys_exit(1);

    while (1) {
    }
}

__attribute__((section(".text.entry"), used))
void p2_start(void) {
    p2_main();

    while (1) {
    }
}
