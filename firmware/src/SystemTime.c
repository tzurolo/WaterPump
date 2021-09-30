//
//  System Time
//
//  Uses AtMega328P 16 bit timer/counter 1
//
#include "SystemTime.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include "StringInteger.h"
#include "Console.h"
#include "EEPROMStorage.h"

#define DEBUG_TRACE 1

static volatile uint8_t tickCounter;
static volatile SystemTime_t currentTime;
static volatile uint32_t secondsSinceStartup;
static int32_t timeAdjustment;
static bool shuttingDown;
static volatile SystemTime_notificationDescriptor *rootNotificationDesc;
#if TICK_STATS
static volatile uint8_t ticksPerMainloop = 0;
static uint8_t minTicksPerMainloop;
static uint8_t maxTicksPerMainloop;
static volatile uint8_t maxCountsPerTick;
#endif

void SystemTime_Initialize (void)
{
    tickCounter = 0;
    currentTime.seconds = 0;
    currentTime.hundredths = 0;
    secondsSinceStartup = 0;
    timeAdjustment = 0;
    shuttingDown = false;
    rootNotificationDesc = NULL;

#if TICK_STATS
    ticksPerMainloop = 0;
    minTicksPerMainloop = 255;
    maxTicksPerMainloop = 0;
    maxCountsPerTick = 0;
#endif

    // set up timer1 to fire interrupt at SYSTEMTIME_TICKS_PER_SECOND
    TCCR1B = (TCCR1B & 0xF8) | 3; // prescale by 64
    TCCR1B = (TCCR1B & 0xE7) | (1 << 3); // set CTC mode
    OCR1A = (F_CPU / 64) / SYSTEMTIME_TICKS_PER_SECOND;
    TCNT1 = 0;  // start the time counter at 0
    TIFR1 |= (1 << OCF1A);  // "clear" the timer compare flag
    TIMSK1 |= (1 << OCIE1A);// enable timer compare match interrupt
}

void SystemTime_registerForTickNotification(
    const uint16_t scaleFactor,
    SystemTime_TickNotificationCB notificationCB,
    void* notificationData,
    SystemTime_notificationDescriptor* notificationDesc)
{
    notificationDesc->next = rootNotificationDesc;
    notificationDesc->scaleFactor = scaleFactor;
    notificationDesc->numTicksRemaining = 1;
    notificationDesc->notificationCB = notificationCB;
    notificationDesc->notificationData = notificationData;
    rootNotificationDesc = notificationDesc;
}

void SystemTime_getCurrentTime (
    SystemTime_t *curTime)
{
    // we disable interrupts during read of secondsSinceReset because
    // it is updated in an interrupt handler
    char SREGSave;
    SREGSave = SREG;
    cli();
    curTime->seconds = currentTime.seconds;
    curTime->hundredths = currentTime.hundredths;
    SREG = SREGSave;
}

uint32_t SystemTime_uptime (void)
{
    uint32_t uptime;

    char SREGSave;
    SREGSave = SREG;
    cli();
    uptime = secondsSinceStartup;
    SREG = SREGSave;

    return uptime;
}

/*
void SystemTime_setTimeAdjustment (
    const uint32_t *newTime)
{
    char SREGSave;
    SREGSave = SREG;
    cli();
    timeAdjustment = *newTime - currentTime.seconds;
    SREG = SREGSave;
#if DEBUG_TRACE
    {
        CharString_define(20, msg);
        CharString_copyP(PSTR("Tadj: "), &msg);
        StringUtils_appendDecimal32(timeAdjustment, 1, 0, &msg);
        Console_printCS(&msg);
    }
#endif
}

void SystemTime_applyTimeAdjustment ()
{
    char SREGSave;

    SREGSave = SREG;
    cli();
    currentTime.seconds += timeAdjustment;
    timeAdjustment = 0;
    SREG = SREGSave;
}

void SystemTime_sleepFor (
    const uint16_t seconds)
{
    uint32_t calibratedSeconds = 
        (((uint32_t)seconds) * EEPROMStorage_watchdogTimerCal()) / 100;
    uint16_t secondsRemaining = calibratedSeconds;
    while (secondsRemaining > 0) {
        uint8_t wdtTimeout;
        uint8_t secondsThisLoop;
        if (secondsRemaining >= 8) {
            wdtTimeout = WDTO_8S;
            secondsThisLoop = 8;
        } else if (secondsRemaining >= 4) {
            wdtTimeout = WDTO_4S;
            secondsThisLoop = 4;
        } else if (secondsRemaining >= 2) {
            wdtTimeout = WDTO_2S;
            secondsThisLoop = 2;
        } else {
            wdtTimeout = WDTO_1S;
            secondsThisLoop = 1;
        }
        secondsRemaining -= secondsThisLoop;
        wdt_enable(wdtTimeout);
        WDTCSR |= (1 << WDIE);
        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        cli();
        sleep_enable();
        sleep_bod_disable();

        sei();
        sleep_cpu();
        sleep_disable();
        sei();
    }
    cli();
    currentTime.seconds += seconds;
    secondsSinceStartup += seconds;
    sei();
}
*/
void SystemTime_commenceShutdown (void)
{
    if (!shuttingDown) {
        shuttingDown = true;
        // store reboot time
        SystemTime_t curTime;
        SystemTime_getCurrentTime(&curTime);
        curTime.seconds += timeAdjustment;    // apply time adjustment
        curTime.seconds += 8;  // account for watchdog time

        Console_printP(PSTR("shutting down..."));
        wdt_enable(WDTO_8S);
        wdt_reset();
    }
}

bool SystemTime_shuttingDown (void)
{
    return shuttingDown;
}

void SystemTime_task (void)
{
    // reset the watchdog timer
    if (shuttingDown) {
//        LED_OUTPORT |= (1 << LED_PIN);
    } else {
        wdt_reset();

        // reboot if it's been more than the stored reboot interval
        // since startup
        const uint32_t uptime = SystemTime_uptime();
        const uint32_t rebootIntervalSeconds =
            (((uint32_t)EEPROMStorage_rebootInterval()) * 60);
        if (uptime > rebootIntervalSeconds) {
            SystemTime_commenceShutdown();
        }

#if TICK_STATS
        char SREGSave;
        SREGSave = SREG;
        cli();
        // update tick count status
        if (ticksPerMainloop < minTicksPerMainloop) {
            minTicksPerMainloop = ticksPerMainloop;
        }
        if (ticksPerMainloop > maxTicksPerMainloop) {
            maxTicksPerMainloop = ticksPerMainloop;
        }
        ticksPerMainloop = 0;
        SREG = SREGSave;
#endif
    }
}

uint8_t SystemTime_timerCounts(void)
{
    return TCNT1;
}

uint8_t SystemTime_dayOfWeek (
    const SystemTime_t *time)
{
    return (time->seconds / 86400L) % 7;
}

uint8_t SystemTime_hours (
    const SystemTime_t *time)
{
    return (time->seconds / 3600) % 24;
}

uint8_t SystemTime_minutes (
    const SystemTime_t *time)
{
    return (time->seconds / 60) % 60;
}

uint8_t SystemTime_seconds (
    const SystemTime_t *time)
{
    return time->seconds % 60;
}

void SystemTime_appendToString (
    const SystemTime_t *time,
    CharString_t* timeString)
{
    // append day of week
    StringInteger_appendDecimal(SystemTime_dayOfWeek(time), 1, 0, timeString);
    CharString_appendP(PSTR(":"), timeString);

    // append hours
    StringInteger_appendDecimal(SystemTime_hours(time), 2, 0, timeString);
    CharString_appendP(PSTR(":"), timeString);

    // append minutes
    StringInteger_appendDecimal(SystemTime_minutes(time), 2, 0, timeString);
    CharString_appendP(PSTR(":"), timeString);

    // append seconds
    StringInteger_appendDecimal(SystemTime_seconds(time), 2, 0, timeString);
}

#if TICK_STATS
void SystemTime_getTickStats(
    uint8_t* minTicks,
    uint8_t* maxTicks,
    uint8_t* maxCounts)
{
    *minTicks = minTicksPerMainloop;
    *maxTicks = maxTicksPerMainloop;
    *maxCounts = maxCountsPerTick;
    minTicksPerMainloop = 255;
    maxTicksPerMainloop = 0;
    maxCountsPerTick = 0;
}
#endif

ISR(TIMER1_COMPA_vect, ISR_BLOCK)
{
    ++tickCounter;
    if (tickCounter >= (SYSTEMTIME_TICKS_PER_SECOND / 100)) {
        tickCounter = 0;
        ++currentTime.hundredths;
        if (currentTime.hundredths >= 100) {
            currentTime.hundredths = 0;
            ++currentTime.seconds;
            ++secondsSinceStartup;
        }
    }

    volatile SystemTime_notificationDescriptor* next = rootNotificationDesc;
    while (next != NULL) {
        if (--next->numTicksRemaining == 0) {
            next->numTicksRemaining = next->scaleFactor;
            next->notificationCB(next->notificationData);
        }
        next = next->next;
    }

#if TICK_STATS
    if (ticksPerMainloop < 255) {
        ++ticksPerMainloop;
    }
    if (TCNT0 > maxCountsPerTick) {
        maxCountsPerTick = TCNT0;
    }
#endif
}

ISR(WDT_vect, ISR_BLOCK)
{
}
