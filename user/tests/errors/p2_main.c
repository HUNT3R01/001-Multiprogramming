#include "user_syscalls.h"

void p2_main(void);

__attribute__((section(".text.entry"), used))
void p2_start(void) {
    p2_main();
    while (1) {
    }
}

void p2_main(void) {
    static const char msg[] = "X\r\n";
    static const char ok[] = "[TEST errors] rc -1 -2 -3 OK\r\n";
    static const char fail[] = "[TEST errors] FAIL\r\n";
    int32_t r_bad_fd;
    int32_t r_bad_ptr;
    int32_t r_bad_id;

    r_bad_fd = sys_write(9, msg, sizeof(msg) - 1);
    r_bad_ptr = sys_write(1, (const void *)0xFFFFFFFF, 8);
    r_bad_id = syscall3(99, 0, 0, 0);

    if (r_bad_fd == -2 && r_bad_ptr == -3 && r_bad_id == -1) {
        (void)sys_write(1, ok, sizeof(ok) - 1);
    } else {
        (void)sys_write(1, fail, sizeof(fail) - 1);
    }

    sys_exit(0);
}
