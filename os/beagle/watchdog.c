#include "os.h"

// Desactiva el watchdog WDT1.
// Esto se hace muy temprano para evitar que la beagle se reinicie sola.
void disable_watchdog(void) {
    PUT32(WDT_WSPR, 0x0000AAAA);
    while (GET32(WDT_WWPS) & WDT_WWPS_PEND) {
    }

    PUT32(WDT_WSPR, 0x00005555);
    while (GET32(WDT_WWPS) & WDT_WWPS_PEND) {
    }
}