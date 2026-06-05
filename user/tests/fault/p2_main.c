#include "user_syscalls.h"

static void force_data_abort(void);
void p2_main(void);

__attribute__((section(".text.entry"), used))
void p2_start(void) {
    p2_main();
    while (1) {
    }
}

void p2_main(void) {
    static const char msg[] = "[TEST fault] P2 forcing data abort\r\n";
    static const char bad[] = "[TEST fault] ERROR P2 should not print\r\n";
    (void)sys_write(1, msg, sizeof(msg) - 1);
    force_data_abort();
    (void)sys_write(1, bad, sizeof(bad) - 1);
    sys_exit(1);
}

static void force_data_abort(void) {
    volatile unsigned int *bad_ptr = (volatile unsigned int *)0xFFFFFFFF;
    volatile unsigned int value;
    value = *bad_ptr;
    (void)value;
}
