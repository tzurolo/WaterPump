//
//  Tachometer and Odometer
//
//  What it does:
//      counts pulses from a sensor on a motor shaft to measure speed
//      and position. Position is simply number of shaft rotations.
//      The sensor is expected to generate pin changes.
//
//  How to use it:
//      define a TachometerOdometer "object" and construct it
//
//      // example - define a tachometer/odometer for sensor on pin PB1
//      TachometerOdometer_t* to;
//      TachometerOdometer_init(IOPortBitfield_ps_b, 1, &to);
//

#ifndef TACHOMETERODOMETER_H
#define TACHOMETERODOMETER_H

#include <stdint.h>
#include <stdbool.h>
#include "PinChangeMonitor.h"
#include "SystemTime.h"

typedef enum TachometerOdometer_direction_enum {
    tod_forward,    // increasing position
    tod_reverse     // decreasing position
} TachometerOdometer_direction_t;

typedef struct TachometerOdometer_struct {
    uint8_t pulsesThisInterval;
    uint8_t speed;
    TachometerOdometer_direction_t dir;
    int16_t position;
    PinChangeMonitor_t sensorPinChanges;
    SystemTime_notificationDescriptor intervalNotification;
} TachometerOdometer_t;

// direction defaults to d_forward
extern void TachometerOdometer_init(
    const IOPortBitfield_PortSelection sensorPort,
    const uint8_t sensorPin,
    TachometerOdometer_t* _this);

extern void TachometerOdometer_setDirection(
    const TachometerOdometer_direction_t dir,
    volatile TachometerOdometer_t* _this);

extern void TachometerOdometer_resetPositionToZero(
    volatile TachometerOdometer_t* _this);

extern TachometerOdometer_direction_t TachometerOdometer_direction(
    volatile TachometerOdometer_t* _this);

extern int16_t TachometerOdometer_position(
    volatile TachometerOdometer_t* _this);

extern uint8_t TachometerOdometer_speed(
    volatile TachometerOdometer_t* _this);

#endif      /* TACHOMETERODOMETER_H */
