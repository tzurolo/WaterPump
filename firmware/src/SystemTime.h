//
//  SystemTime
//
//  Counts seconds since last reset
//  Resets the watchdog timer
//
//  Uses AtMega328P 8 bit timer/counter 0
//
#ifndef SYSTEMTIME_H
#define SYSTEMTIME_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include "SystemTimeCommon.h"
#include "CharString.h"

#define SYSTEMTIME_TICKS_PER_SECOND 4800
#define COUNTS_PER_TICK (F_CPU / 64) / SYSTEMTIME_TICKS_PER_SECOND

#define TICK_STATS 0

// how the last reboot occurred
typedef enum SystemTime_LastRebootBy_enum {
    lrb_software,   // reboot initiated by software (e.g. reboot command)
    lrb_hardware    // reboot initiated by hardware (e.g. reset line pulled low)
} SystemTime_LastRebootBy;

// prototype for functions that clients supply to
// get notification when a tick occurs
typedef void (*SystemTime_TickNotificationCB)(void*);

extern void SystemTime_Initialize (void);

// note that your notification function will be
// called from an interrupt handler
typedef struct SystemTime_notificationStruct {
    volatile struct SystemTime_notificationStruct* next;
    uint16_t scaleFactor;
    uint16_t numTicksRemaining;
    SystemTime_TickNotificationCB notificationCB;
    void* notificationData;
} SystemTime_notificationDescriptor;
extern void SystemTime_registerForTickNotification(
    const uint16_t scaleFactor, // notify on every scaleFactor number of ticks
    SystemTime_TickNotificationCB notificationCB,
    void* notificationData,     // to be passed to notificationFcn
    SystemTime_notificationDescriptor* notificationDesc);

extern uint32_t SystemTime_uptime (void);

// this function is used to resynchronize system time to
// server time, but not immediately. This function stores
// an offset from the given time to the current system time.
// The adjustment takes effect when you call
// SystemTime_applyTimeAdjustment()
extern void SystemTime_setTimeAdjustment (
    const uint32_t *newTime);
extern void SystemTime_applyTimeAdjustment (void);

// sleep for up to 18 hours
extern void SystemTime_sleepFor (
    const uint16_t seconds);

extern void SystemTime_task (void);

extern uint8_t SystemTime_timerCounts(void);

// returns t1.seconds - t2.seconds
inline int32_t SystemTime_diffSec (
    const SystemTime_t *t1,
    const SystemTime_t *t2)
{
    return t1->seconds - t2->seconds;
}

extern uint8_t SystemTime_dayOfWeek (
    const SystemTime_t *time);
extern uint8_t SystemTime_hours (
    const SystemTime_t *time);
extern uint8_t SystemTime_minutes (
    const SystemTime_t *time);
extern uint8_t SystemTime_seconds (
    const SystemTime_t *time);

// writes given time as D:HH:MM:SS
extern void SystemTime_appendToString (
    const SystemTime_t *time,
    CharString_t* timeString);

#if TICK_STATS
// gets tick stats and resets them
extern void SystemTime_getTickStats(
    uint8_t* minTicks,
    uint8_t* maxTicks,
    uint8_t* maxCounts);
#endif

#endif  // SYSTEMTIME_H
