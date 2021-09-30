//
//  LinearMotion Control
//
//  This unit controls a DC gearmotor connected to a threaded rod. A nut on
//  the threaded rod provides linear motion.
//  The gearmotor is has a tachometer / odomoter, and there is a home position
//  sensor that detects when the nut reaches the home position.
//

#include "LinearMotionControl.h"

#include "SystemTime.h"
#include "Console.h"
#include "StringInteger.h"
#include <avr/io.h>

#include "Console.h"

#define DEBUG_TRACE 0

#define TARGET_POSITION_TIMEOUT_TIME 2000

#define M1A_PIN PD5
#define M1A_PORT PORTD
#define M1A_DIR DDRD
#define M1B_PIN PD6
#define M1B_PORT PORTD
#define M1B_DIR DDRD

static void setupPhaseCorrectPWM(void) {
    TCCR0A = (TCCR0A & 0x3F) | 2 << COM0A0; // set to Clear OC0A on Compare Match when up-counting
    TCCR0A = (TCCR0A & 0xCF) | 2 << COM0B0; // set to Clear OC0B on Compare Match when up-counting
    TCCR0A = (TCCR0A & 0xFC) | 1 << WGM00;  // set PWM, Phase Correct bits WGM00 and WGM01
    TCCR0B = (TCCR0B & 0xF7) | 0 << WGM02;  // set PWM, Phase Correct bits WGM02
    TCCR0B = (TCCR0B & 0xF8) | 3 << CS00;   // set prescaler to div 64
}
static void motorForward(const uint8_t motorPWM) {
#if 0
    M1A_PORT |= (1 << M1A_PIN);
    M1B_PORT &= ~(1 << M1B_PIN);
#else
#if DEBUG_TRACE
    CharString_define(50, msg);
    CharString_copyP(PSTR("fwd pwm: "), &msg);
    StringInteger_appendDecimal(motorPWM, 0, 0, &msg);
    Console_printLineCS(&msg);
#endif
    setupPhaseCorrectPWM();
    // pwm pin OC0B (PD5)
    OCR0A = 0;
    OCR0B = motorPWM;
#endif
}

static void motorReverse(const uint8_t motorPWM) {
#if 0
    M1A_PORT &= ~(1 << M1A_PIN);
    M1B_PORT |= (1 << M1B_PIN);
#else
#if DEBUG_TRACE
    CharString_define(50, msg);
    CharString_copyP(PSTR("rev pwm: "), &msg);
    StringInteger_appendDecimal(motorPWM, 0, 0, &msg);
    Console_printLineCS(&msg);
#endif
    // pwm pin OCR0A (PD6)
    setupPhaseCorrectPWM();
    OCR0A = motorPWM;
    OCR0B = 0;
#endif
}

static void motorBrake (void) {
    // turn off pwm
    TCCR0A = 0;
    TCCR0B = 0;
    // turn on MIA and M1B
    M1A_PORT |= (1 << M1A_PIN);
    M1B_PORT |= (1 << M1B_PIN);
}

static void motorCoast(void) {
    TCCR0A = 0;
    TCCR0B = 0;
    // turn off MIA and M1B
    M1A_PORT &= ~(1 << M1A_PIN);
    M1B_PORT &= ~(1 << M1B_PIN);
}

static void homePositionSensorChangeCB(
    const bool pinState,
    void* clientData)
{
    LinearMotionControl_t* lmc = (LinearMotionControl_t*)clientData;
    TachometerOdometer_resetPositionToZero(&lmc->to);
    lmc->foundHomePosition = true;
    // Console_printLineP(PSTR("home"));
}

void LinearMotionControl_init(
    const IOPortBitfield_PortSelection tachometerOdometerPort,
    const uint8_t tachometerOdometerPin,
    const IOPortBitfield_PortSelection homePositionSensorPort,
    const uint8_t homePositionSensorPin,
    LinearMotionControl_t* _this)
{
    _this->command = lmcc_none;
    _this->targetPosition = 0;
    _this->motorPWM = 0;
    _this->state = lmcs_stopped;
    TachometerOdometer_init(tachometerOdometerPort, tachometerOdometerPin, &_this->to);
    IOPortBitfield_init(homePositionSensorPort, homePositionSensorPin, 1, false,
        &_this->homePositionSensorInput);
    PinChangeMonitor_monitorPin(homePositionSensorPort, homePositionSensorPin,
        homePositionSensorChangeCB, _this,
        &_this->homePositionSensorInputChangeMonitor);
    PinChangeMonitor_enable(&_this->homePositionSensorInputChangeMonitor);
    _this->foundHomePosition = false;

    // for small movements it is possible that we reach the
    // target position before speed is computed by the tachometer.
    // In this case we read zero speed and may falsely conclude that
    // the motor has stopped. We need to wait until we have seen
    // a nonzero speed before we accept a speed of zero as an
    // indication that the motor has stopped
    _this->hadNonzeroSpeed = false;

    // set up motor driver pins - make them outputs
    M1A_DIR |= (1 << M1A_PIN);
    M1B_DIR |= (1 << M1B_PIN);
    motorCoast();
}

bool LinearMotionControl_moveToPosition(
    const int16_t newPosition,
    const uint8_t motorPWM,
    LinearMotionControl_t* _this)
{
    if (_this->foundHomePosition) {
        _this->command = lmcc_moveToPosition;
        _this->targetPosition = newPosition;
        _this->motorPWM = motorPWM;
        return true;
    }
    return false;
}

void LinearMotionControl_brakeToStop(
    LinearMotionControl_t* _this)
{
    _this->command = lmcc_brakeToStop;
}

bool LinearMotionControl_isStopped(
    LinearMotionControl_t* _this)
{
    return _this->state == lmcs_stopped;
}

void LinearMotionControl_findHomePosition(
    const uint8_t motorPWM,
    LinearMotionControl_t* _this)
{
    _this->foundHomePosition = false;
    _this->command = lmcc_findHomePosition;
    _this->motorPWM = motorPWM;
}

int16_t LinearMotionControl_position(
    LinearMotionControl_t* _this)
{
    return TachometerOdometer_position(&_this->to);
}
uint8_t LinearMotionControl_speed(
    LinearMotionControl_t* _this)
{
    return TachometerOdometer_speed(&_this->to);
}

bool LinearMotionControl_homePositionIsKnown(
    LinearMotionControl_t* _this)
{
    return _this->foundHomePosition;
}

static void brakeToStop(
    LinearMotionControl_t* _this)
{
    motorBrake();
    _this->state = lmcs_brakingToStop;
#if DEBUG_TRACE
    Console_printLineP(PSTR("braking"));
#endif
}

void LinearMotionControl_task(
    LinearMotionControl_t* _this)
{
    switch (_this->state) {
        case lmcs_stopped:
            switch (_this->command) {
                case lmcc_moveToPosition: {
                    // see where we are relative to new position
                    const int16_t currentPosition = TachometerOdometer_position(&_this->to);
                    if (_this->targetPosition > currentPosition) {
                        // move forward
                        TachometerOdometer_setDirection(tod_forward, &_this->to);
                        motorForward(_this->motorPWM);
                        SystemTime_futureTime(TARGET_POSITION_TIMEOUT_TIME, &_this->timeoutTimer);
                        _this->state = lmcs_movingToPosition;
                        _this->hadNonzeroSpeed = false;
                    } else if (_this->targetPosition < currentPosition) {
                        // move reverse
                        TachometerOdometer_setDirection(tod_reverse, &_this->to);
                        motorReverse(_this->motorPWM);
                        SystemTime_futureTime(TARGET_POSITION_TIMEOUT_TIME, &_this->timeoutTimer);
                        _this->state = lmcs_movingToPosition;
                        _this->hadNonzeroSpeed = false;
                    }
                    }
                    break;
                case lmcc_findHomePosition:
                    if (IOPortBitfield_readAsBool(&_this->homePositionSensorInput)) {
                        // carriage position is currently ahead of home position
                        // search in reverse
                        TachometerOdometer_setDirection(tod_reverse, &_this->to);
                        motorReverse(_this->motorPWM);
                    } else {
                        // carriage position is currently behind home position
                        // search forward
                        TachometerOdometer_setDirection(tod_forward, &_this->to);
                        motorForward(_this->motorPWM);
                    }
                    SystemTime_futureTime(TARGET_POSITION_TIMEOUT_TIME, &_this->timeoutTimer);
                    _this->state = lmcs_searchingForHomePosition;
                    break;
                default:
                    break;
            }
            _this->command = lmcc_none;
            break;
        case lmcs_movingToPosition: {
            const TachometerOdometer_direction_t dir =
                TachometerOdometer_direction(&_this->to);
            const int16_t pos = TachometerOdometer_position(&_this->to);
            const uint8_t speed = TachometerOdometer_speed(&_this->to);
            if (speed != 0) {
                _this->hadNonzeroSpeed = true;
            }
            if ((_this->command == lmcc_brakeToStop) ||
                ((dir == tod_forward) && (pos >= _this->targetPosition)) ||
                ((dir == tod_reverse) && (pos <= _this->targetPosition))) {
#if DEBUG_TRACE
                CharString_define(40, msg);
                CharString_appendP(PSTR("reached "), &msg);
                StringInteger_appendDecimal(pos, 1, 0, &msg);
                CharString_appendP(PSTR(", target: "), &msg);
                StringInteger_appendDecimal(_this->targetPosition, 1, 0, &msg);
                CharString_appendP(PSTR(", speed: "), &msg);
                StringInteger_appendDecimal(speed, 1, 0, &msg);
                Console_printLineCS(&msg);
#endif
                if (_this->command == lmcc_brakeToStop) {
                    _this->command = lmcc_none;
                }
                brakeToStop(_this);
            } else if (SystemTime_timeHasArrived(&_this->timeoutTimer)) {
                motorCoast();
                _this->state = lmcs_stopped;
            }
            }
            break;
        case lmcs_brakingToStop: {
            const uint8_t speed = TachometerOdometer_speed(&_this->to);
            if (speed != 0) {
                _this->hadNonzeroSpeed = true;
            }
            if ((speed == 0) && _this->hadNonzeroSpeed) {
                motorCoast();
#if DEBUG_TRACE
                CharString_define(40, msg);
                CharString_appendP(PSTR("stopped at "), &msg);
                StringInteger_appendDecimal(
                    TachometerOdometer_position(&_this->to), 1, 0, &msg);
                Console_printLineCS(&msg);
#endif
                _this->state = lmcs_stopped;
            }
            }
            break;
        case lmcs_searchingForHomePosition:
            if (TachometerOdometer_speed(&_this->to) != 0) {
                _this->hadNonzeroSpeed = true;
            }
            if (_this->foundHomePosition ||
                (_this->command == lmcc_brakeToStop)) {
#if DEBUG_TRACE
                CharString_define(40, msg);
                CharString_appendP(PSTR("homing speed: "), &msg);
                StringInteger_appendDecimal(
                    TachometerOdometer_speed(&_this->to), 1, 0, &msg);
                Console_printLineCS(&msg);
#endif
                brakeToStop(_this);
            } else if (SystemTime_timeHasArrived(&_this->timeoutTimer)) {
                motorCoast();
                _this->state = lmcs_stopped;
            }
            break;
        default:
            break;
    }
}

