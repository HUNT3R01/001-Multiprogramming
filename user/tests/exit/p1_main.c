#include "user_syscalls.h"

void p1_main(void);

__attribute__((section(".text.entry"), used))
void p1_start(void) {
    p1_main();
    while (1) {
    }
}

void p1_main(void) {
    static const char start[] = "[TEST exit] P1 gives CPU to P2\r\n";
    static const char end[] = "[TEST exit] P1 still alive after P2 exit\r\n";
    (void)sys_write(1, start, sizeof(start) - 1);
    (void)sys_yield();
    (void)sys_write(1, end, sizeof(end) - 1);
    sys_exit(0);
}
