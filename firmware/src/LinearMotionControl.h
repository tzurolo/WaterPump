//
//  Linear Motion Control
//
#ifndef LINEARMOTIONCONTROL_H
#define LINEARMOTIONCONTROL_H

#include <stdint.h>
#include <stdbool.h>
#include "TachometerOdometer.h"
#include "IOPortBitfield.h"
#include "PinChangeMonitor.h"
#include "SystemTime.h"

typedef enum {
    lmcc_none,
    lmcc_moveToPosition,
    lmcc_brakeToStop,
    lmcc_findHomePosition
} LinearMotionControl_command;

typedef enum {
    lmcs_stopped,
    lmcs_movingToPosition,
    lmcs_brakingToStop,
    lmcs_searchingForHomePosition
} LinearMotionControl_state;

typedef struct LinearMotionControl_struct {
    LinearMotionControl_command command;
    int16_t targetPosition;
    uint8_t motorPWM;
    LinearMotionControl_state state;
    TachometerOdometer_t to;
    IOPortBitfield_t homePositionSensorInput;
    PinChangeMonitor_t homePositionSensorInputChangeMonitor;
    bool foundHomePosition;
    bool hadNonzeroSpeed;
    SystemTime_t timeoutTimer;
} LinearMotionControl_t;

extern void LinearMotionControl_init(
    const IOPortBitfield_PortSelection tachometerOdometerPort,
    const uint8_t tachometerOdometerPin,
    const IOPortBitfield_PortSelection homePositionSensorPort,
    const uint8_t homePositionSensorPin,
    LinearMotionControl_t* _this);

extern bool LinearMotionControl_moveToPosition(
    const int16_t newPosition,
    const uint8_t motorPWM,    // 0 to 255
    LinearMotionControl_t* _this);

extern void LinearMotionControl_brakeToStop(
    LinearMotionControl_t* _this);

extern bool LinearMotionControl_isStopped(
    LinearMotionControl_t* _this);

extern void LinearMotionControl_findHomePosition(
    const uint8_t motorPWM,    // 0 to 255
    LinearMotionControl_t* _this);

extern int16_t LinearMotionControl_position(
    LinearMotionControl_t* _this);
extern uint8_t LinearMotionControl_speed(
    LinearMotionControl_t* _this);

extern bool LinearMotionControl_homePositionIsKnown (
    LinearMotionControl_t* _this);

extern void LinearMotionControl_task(
    LinearMotionControl_t* _this);

#endif  /* LINEARMOTIONCONTROL_H */