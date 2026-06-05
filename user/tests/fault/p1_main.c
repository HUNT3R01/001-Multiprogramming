#include "user_syscalls.h"

void p1_main(void);

__attribute__((section(".text.entry"), used))
void p1_start(void) {
    p1_main();
    while (1) {
    }
}

void p1_main(void) {
    static const char msg[] = "[TEST fault] P1 alive\r\n";
    int i;

    for (i = 0; i < 4; i++) {
        (void)sys_write(1, msg, sizeof(msg) - 1);
        (void)sys_yield();
    }

    sys_exit(0);
}
