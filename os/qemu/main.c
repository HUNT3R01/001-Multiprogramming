#include "os.h"

int main(void) {
    os_puts("\r\n=== OS Multiprogramming QEMU ===\r\n");
    os_puts("Inicializando PCBs...\r\n");

    init_pcbs();
    os_puts("PCBs listos\r\n");

    timer_init();
    os_puts("Timer listo\r\n");

    os_puts("Iniciando P1...\r\n");

    current_process = 1;
    start_first_process(pcb[1].sp);

    while (1) {
    }

    return 0;
}