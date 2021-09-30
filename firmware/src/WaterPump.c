//
//  Water Pump Controller
//
//  This program controls a water pump that uses a linear actuator to push
//  and pull a syringe plunger.
//
//
#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include "intlimit.h"
#include "SystemTime.h"
#include "EEPROMStorage.h"
#include "Console.h"
#include "PinChangeMonitor.h"
#include "WaterPumpControl.h"
#include "RAMSentinel.h"

/** Configures the board hardware and chip peripherals for the demo's functionality. */
static void Initialize (void)
{
    // enable watchdog timer
    wdt_enable(WDTO_500MS);

    SystemTime_Initialize();
    EEPROMStorage_Initialize();
    Console_Initialize();
    PinChangeMonitor_Initialize();
    WaterPumpControl_Initialize();
    RAMSentinel_Initialize();
}
 
int main (void)
{
    Initialize();

    sei();

    for (;;) {
        // run all the tasks
        SystemTime_task();
        WaterPumpControl_task();
        Console_task();

        if (!RAMSentinel_sentinelIntact()) {
            SystemTime_commenceShutdown();
        }

#if COUNT_MAJOR_CYCLES
        ++majorCycleCounter;
#endif
    }

    return (0);
}
