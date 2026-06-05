#include "user_syscalls.h"

void p2_main(void);

__attribute__((section(".text.entry"), used))
void p2_start(void) {
    p2_main();
    while (1) {
    }
}

void p2_main(void) {
    static const char msg[] = "[TEST write] P2 write OK\r\n";
    (void)sys_write(1, msg, sizeof(msg) - 1);
    sys_exit(0);
}
