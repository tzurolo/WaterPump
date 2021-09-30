//
//  Tachometer and Odometer
//
//  How it works:
//      Registers for pin change notification on the sensor pin to detect
//      shaft motion. Also registers for system tick notification to get
//      speed (the number of sensor pin pulses during a tick notification
//      interval is the speed). The interval is 200mS
//

#include "TachometerOdometer.h"

#include <avr/interrupt.h>

static void pinChangeNotificationCB(
    const bool pinState,
    void* clientData)
{
    if (!pinState) {    // only counting falling edges
        TachometerOdometer_t* to = (TachometerOdometer_t*)clientData;
        if (to->pulsesThisInterval < 255) {
            ++to->pulsesThisInterval;
        }
        if (to->dir == tod_forward) {
            ++to->position;
        } else {
            --to->position;
        }
    }
}

void intervalNotificationCB(
    void* clientData)
{
    TachometerOdometer_t* to = (TachometerOdometer_t*)clientData;
    to->speed = to->pulsesThisInterval;
    to->pulsesThisInterval = 0;
}

void TachometerOdometer_init(
    const IOPortBitfield_PortSelection sensorPort,
    const uint8_t sensorPin,
    TachometerOdometer_t* _this)
{
    _this->pulsesThisInterval = 0;
    _this->speed = 0;
    _this->dir = tod_forward;
    _this->position = 0;
    PinChangeMonitor_monitorPin(sensorPort, sensorPin,
        pinChangeNotificationCB, _this, &_this->sensorPinChanges);
    PinChangeMonitor_enable(&_this->sensorPinChanges);
    SystemTime_registerForTickNotification(SYSTEMTIME_TICKS_PER_SECOND / 5,
        intervalNotificationCB, _this, &_this->intervalNotification);
}

void TachometerOdometer_setDirection(
    const TachometerOdometer_direction_t dir,
    volatile TachometerOdometer_t* _this)
{
    // we disable interrupts during setting of direction because
    // it is read in an interrupt handler
    char SREGSave;
    SREGSave = SREG;
    cli();
    _this->dir = dir;
    SREG = SREGSave;
}

void TachometerOdometer_resetPositionToZero(
    volatile TachometerOdometer_t* _this)
{
    // we disable interrupts during clearing of position because
    // it is updated in an interrupt handler
    char SREGSave;
    SREGSave = SREG;
    cli();
    _this->position = 0;
    SREG = SREGSave;
}

TachometerOdometer_direction_t TachometerOdometer_direction(
    volatile TachometerOdometer_t* _this)
{
    return _this->dir;
}

int16_t TachometerOdometer_position(
    volatile TachometerOdometer_t* _this)
{
    int16_t pos;
    // we disable interrupts during read of position because
    // it is updated in an interrupt handler
    char SREGSave;
    SREGSave = SREG;
    cli();
    pos = _this->position;
    SREG = SREGSave;
    return pos;
}

uint8_t TachometerOdometer_speed(
    volatile TachometerOdometer_t* _this)
{
    uint8_t speed;
    // we disable interrupts during read of speed because
    // it is updated in an interrupt handler
    char SREGSave;
    SREGSave = SREG;
    cli();
    speed = _this->speed;
    SREG = SREGSave;
    return speed;
}
