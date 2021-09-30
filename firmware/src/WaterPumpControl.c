//
//  Water Pump Control
//
//  Uses pin PC4 for water level float sensor (QTR-1A reflectance sensor)
//
#include "WaterPumpControl.h"

#include "avr/io.h"
#include "LinearMotionControl.h"
#include "EEPROMStorage.h"

#include "Console.h"
#include "StringInteger.h"
#define DEBUG_TRACE 1

#define FLOAT_SENSOR_DIR DDRC
#define FLOAT_SENSOR_INPORT PINC
#define FLOAT_SENSOR_OUTPORT PORTC
#define FLOAT_SENSOR_PIN PC4

typedef enum pumpingState_enum {
    ps_idle,
    ps_findingHomePosition,
    ps_drawingWaterIn,
    ps_pushingWaterOut
} pumpingState;

static bool floatSensorLast;

static pumpingState state;
static bool runPump;
static uint16_t volumeRemainingToPump;   // units: ml
static int16_t plungerOutPosition;
static LinearMotionControl_t syringePlunger;

// returns true when the float sensor is actuated (float ball in range)
static bool readFloatSensor(void)
{
    return (FLOAT_SENSOR_INPORT & (1 << FLOAT_SENSOR_PIN)) == 0;
}

void WaterPumpControl_Initialize(void)
{
    // set up float sensor pin
    FLOAT_SENSOR_DIR &= ~(1 << FLOAT_SENSOR_PIN);
    // enable pull-up in case sensor is disconnected
    FLOAT_SENSOR_OUTPORT |= (1 << FLOAT_SENSOR_PIN);
    floatSensorLast = false;

    state = ps_idle;
    runPump = false;
    volumeRemainingToPump = 0;

    LinearMotionControl_init(
        IOPortBitfield_ps_b, 0, // tachometer/odomerter sensor pin
        IOPortBitfield_ps_d, 2, // home position sensor pin
        &syringePlunger);
}

void WaterPumpControl_beginPumping(void)
{
    if (!runPump) {
#if DEBUG_TRACE
        Console_printLineP(PSTR("starting pump"));
#endif
        volumeRemainingToPump = EEPROMStorage_mlToPump();
        runPump = true;
    }
}

void WaterPumpControl_endPumping(void)
{
    runPump = false;
}

void WaterPumpControl_stopNow(void)
{
    LinearMotionControl_brakeToStop(&syringePlunger);
    runPump = false;
    state = ps_idle;
}

void WaterPumpControl_movePlungerTo(
    const int16_t pos)
{
    LinearMotionControl_moveToPosition(pos, EEPROMStorage_motorPwm(), &syringePlunger);
}

int16_t WaterPumpControl_plungerPosition(void)
{
    return LinearMotionControl_position(&syringePlunger);
}

uint8_t WaterPumpControl_plungerSpeed(void)
{
    return LinearMotionControl_speed(&syringePlunger);
}

uint16_t WaterPumpControl_volumeRemaining(void)
{
    return volumeRemainingToPump;
}

void WaterPumpControl_task(void)
{
    // check float sensor
    const bool floatSensor = readFloatSensor();
    if (floatSensor != floatSensorLast) {
        // float sensor state changed
        floatSensorLast = floatSensor;

        if (floatSensor) {
            // float sensor actuated
            WaterPumpControl_beginPumping();
        }
    }

    switch (state) {
        case ps_idle:
            if (runPump) {
                if (!LinearMotionControl_homePositionIsKnown(&syringePlunger)) {
                    LinearMotionControl_findHomePosition(EEPROMStorage_motorPwm(), &syringePlunger);
                    state = ps_findingHomePosition;
                } else {
                    LinearMotionControl_moveToPosition(
                        EEPROMStorage_plungerOutPos(), EEPROMStorage_motorPwm(), &syringePlunger);
                    state = ps_drawingWaterIn;
                }
            }
            break;
        case ps_findingHomePosition:
            if (LinearMotionControl_homePositionIsKnown(&syringePlunger) &&
                LinearMotionControl_isStopped(&syringePlunger)) {
                LinearMotionControl_moveToPosition(
                    EEPROMStorage_plungerOutPos(), EEPROMStorage_motorPwm(), &syringePlunger);
                state = ps_drawingWaterIn;
            }
            break;
        case ps_drawingWaterIn:
            if (LinearMotionControl_isStopped(&syringePlunger) &&
                (LinearMotionControl_position(&syringePlunger) <=
                    EEPROMStorage_plungerOutPos())) {
                plungerOutPosition = LinearMotionControl_position(&syringePlunger);
                LinearMotionControl_moveToPosition(
                    EEPROMStorage_plungerInPos(), EEPROMStorage_motorPwm(), &syringePlunger);
                state = ps_pushingWaterOut;
            }
            break;
        case ps_pushingWaterOut:
            if (LinearMotionControl_isStopped(&syringePlunger) &&
                (LinearMotionControl_position(&syringePlunger) >=
                    EEPROMStorage_plungerInPos())) {
                const int16_t plungerTravel =
                    LinearMotionControl_position(&syringePlunger) - plungerOutPosition;
                const uint16_t volumePumped = plungerTravel / EEPROMStorage_posPerMl();

                if (volumePumped > volumeRemainingToPump) {
                    volumeRemainingToPump = 0;
                    runPump = false;
                } else {
                    volumeRemainingToPump -= volumePumped;
                }
#if DEBUG_TRACE
                CharString_define(40, msg);
                CharString_appendP(PSTR("pumped "), &msg);
                StringInteger_appendDecimal(volumePumped, 1, 0, &msg);
                CharString_appendP(PSTR(" ml"), &msg);
                Console_printLineCS(&msg);
#endif
                if (runPump) {
                    LinearMotionControl_moveToPosition(
                        EEPROMStorage_plungerOutPos(), EEPROMStorage_motorPwm(), &syringePlunger);
                    state = ps_drawingWaterIn;
                } else {
                    state = ps_idle;
                }
            }
            break;
    }

    LinearMotionControl_task(&syringePlunger);
}
