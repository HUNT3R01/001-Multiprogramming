#include "user_syscalls.h"

void p2_main(void);

__attribute__((section(".text.entry"), used))
void p2_start(void) {
    p2_main();
    while (1) {
    }
}

void p2_main(void) {
    static const char msg[] = "[TEST exit] P2 exiting now\r\n";
    static const char bad[] = "[TEST exit] ERROR P2 should not print\r\n";
    (void)sys_write(1, msg, sizeof(msg) - 1);
    sys_exit(0);
    (void)sys_write(1, bad, sizeof(bad) - 1);
}
