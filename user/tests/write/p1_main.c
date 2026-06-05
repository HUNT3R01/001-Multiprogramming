#include "user_syscalls.h"

void p1_main(void);

__attribute__((section(".text.entry"), used))
void p1_start(void) {
    p1_main();
    while (1) {
    }
}

void p1_main(void) {
    static const char msg[] = "[TEST write] P1 write OK\r\n";
    (void)sys_write(1, msg, sizeof(msg) - 1);
    sys_exit(0);
}
