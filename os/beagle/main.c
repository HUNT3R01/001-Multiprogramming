#include "os.h"

/*
   Main del OS para BeagleBone Black.
   Inicializa hardware, PCBs y timer.
   Luego arranca el primer proceso.
*/
int main(void) {
    disable_watchdog();

    os_puts("\r\n=== OS Multiprogramming ===\r\n");
    os_puts("Target: BeagleBone Black\r\n");

    init_pcbs();
    os_puts("PCBs y stacks inicializados\r\n");

    timer_init();
    os_puts("Timer listo\r\n");

    os_puts("Listo - iniciando P1\r\n");

    current_process = 1;
    start_first_process(pcb[1].sp);

    while (1) {
    }

    return 0;
}