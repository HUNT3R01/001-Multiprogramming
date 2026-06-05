#include "user_syscalls.h"

void p2_main(void);

__attribute__((section(".text.entry"), used))
void p2_start(void) {
    p2_main();
    while (1) {
    }
}

void p2_main(void) {
    static const char msg[] = "[TEST yield] P2 ran after P1 yield\r\n";
    (void)sys_write(1, msg, sizeof(msg) - 1);
    (void)sys_yield();
    sys_exit(0);
}
