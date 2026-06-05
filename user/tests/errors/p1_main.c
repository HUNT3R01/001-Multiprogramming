#include "user_syscalls.h"

void p1_main(void);

__attribute__((section(".text.entry"), used))
void p1_start(void) {
    p1_main();
    while (1) {
    }
}

void p1_main(void) {
    static const char msg[] = "[TEST errors] P1 yield to P2\r\n";
    (void)sys_write(1, msg, sizeof(msg) - 1);
    (void)sys_yield();
    sys_exit(0);
}
