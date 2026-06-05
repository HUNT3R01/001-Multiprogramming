#include "user_syscalls.h"

void p1_main(void);

__attribute__((section(".text.entry"), used))
void p1_start(void) {
    p1_main();
    while (1) {
    }
}

void p1_main(void) {
    static const char before[] = "[TEST yield] P1 before yield\r\n";
    static const char after[] = "[TEST yield] P1 after yield\r\n";
    (void)sys_write(1, before, sizeof(before) - 1);
    (void)sys_yield();
    (void)sys_write(1, after, sizeof(after) - 1);
    sys_exit(0);
}
