//
// Command processor
//
// Interprets and executes commands from the console
//

#include "CommandProcessor.h"

#include <stdlib.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include "SystemTime.h"
#include "Console.h"
#include "CharString.h"
#include "StringScan.h"
#include "StringInteger.h"
#include "EEPROM_Util.h"
#include "EEPROMStorage.h"
#include "WaterPumpControl.h"
#include "MSVS_AVR.h"

#include <avr/io.h> // only for PWM test


const char swver[] PROGMEM = "V1.0";

static const char motorPwmP[]     PROGMEM = "motorPwm";
static const char tCalOffsetP[]   PROGMEM = "tCalOffset";
static const char posPerMlP[]     PROGMEM = "posPerMl";
static const char inPosP[]        PROGMEM = "inPos";
static const char outPosP[]       PROGMEM = "outPos";
static const char mlToPumpP[]     PROGMEM = "mlToPump";

CharString_define(80, CommandProcessor_incomingCommand)
CharString_define(100, CommandProcessor_commandReply)

static int16_t scanIntegerToken(
    CharStringSpan_t* str,
    bool* isValid)
{
    StringScan_skipWhitespace(str);
    int16_t value = 0;
    StringInteger_scan(str, isValid, &value, str);
    return value;
}

static void beginJSON(
    CharString_t* str)
{
    CharString_copyP(PSTR("{"), str);
}

static void continueJSON(
    CharString_t* str)
{
    CharString_appendC(',', str);
}

static void endJSON(
    CharString_t* str)
{
    CharString_appendC('}', str);
}

static void appendJSONIntValue(
    PGM_P name,
    const int16_t value,
    const uint8_t decimalPlaces,
    CharString_t* str)
{
    CharString_appendC('\"', str);
    CharString_appendP(name, str);
    CharString_appendP(PSTR("\":"), str);
    StringInteger_appendDecimal(value, 1, decimalPlaces, str);
}

static void appendJSONTimeValue(
    PGM_P name,
    const SystemTime_t* time,
    CharString_t* str)
{
    CharString_appendC('\"', str);
    CharString_appendP(name, str);
    CharString_appendP(PSTR("\":\""), str);
    SystemTime_appendToString(time, str);
    CharString_appendC('\"', str);
}

bool CommandProcessor_executeCommand(
    const CharStringSpan_t* command,
    CharString_t* reply)
{
    bool validCommand = true;

    CharStringSpan_t cmd = *command;
    CharStringSpan_t cmdToken;
    StringScan_scanToken(&cmd, &cmdToken);
    if (CharStringSpan_equalsNocaseP(&cmdToken, PSTR("s"))) {
        SystemTime_t curTime;
        SystemTime_getCurrentTime(&curTime);
        beginJSON(reply);
        appendJSONTimeValue(PSTR("t"), &curTime, reply);
        continueJSON(reply);
        appendJSONIntValue(PSTR("pos"), WaterPumpControl_plungerPosition(), 0, reply);
        continueJSON(reply);
        appendJSONIntValue(PSTR("speed"), WaterPumpControl_plungerSpeed(), 0, reply);
        continueJSON(reply);
        appendJSONIntValue(PSTR("volumeRemaining"), WaterPumpControl_volumeRemaining(), 0, reply);
        endJSON(reply);
    } else if (CharStringSpan_equalsNocaseP(&cmdToken, PSTR("settings"))) {
        CharString_define(16, settingStr);
        Console_printP(PSTR("{"));
        CharString_copyP(PSTR("}"), &settingStr);
        Console_printLineCS(&settingStr);
    } else if (CharStringSpan_equalsNocaseP(&cmdToken, PSTR("set"))) {
        StringScan_scanToken(&cmd, &cmdToken);
        if (CharStringSpan_equalsNocaseP(&cmdToken, tCalOffsetP)) {
            const int16_t tempCalOffset = scanIntegerToken(&cmd, &validCommand);
            if (validCommand) {
                EEPROMStorage_setTempCalOffset(tempCalOffset);
            }
        } else if (CharStringSpan_equalsNocaseP(&cmdToken, inPosP)) {
            const int16_t pos = scanIntegerToken(&cmd, &validCommand);
            if (validCommand) {
                EEPROMStorage_setPlungerInPos(pos);
            }
        } else if (CharStringSpan_equalsNocaseP(&cmdToken, outPosP)) {
            const int16_t pos = scanIntegerToken(&cmd, &validCommand);
            if (validCommand) {
                EEPROMStorage_setPlungerOutPos(pos);
            }
        } else if (CharStringSpan_equalsNocaseP(&cmdToken, posPerMlP)) {
            const uint16_t posPerMl = scanIntegerToken(&cmd, &validCommand);
            if (validCommand) {
                EEPROMStorage_setPosPerMl(posPerMl);
            }
        } else if (CharStringSpan_equalsNocaseP(&cmdToken, mlToPumpP)) {
            const uint16_t mlToPump = scanIntegerToken(&cmd, &validCommand);
            if (validCommand) {
                EEPROMStorage_setMlToPump(mlToPump);
            }
        } else if (CharStringSpan_equalsNocaseP(&cmdToken, motorPwmP)) {
            const uint8_t pwm = scanIntegerToken(&cmd, &validCommand);
            if (validCommand) {
                EEPROMStorage_setMotorPwm(pwm);
            }
        } else {
            validCommand = false;
        }
    } else if (CharStringSpan_equalsNocaseP(&cmdToken, PSTR("get"))) {
        StringScan_scanToken(&cmd, &cmdToken);
        if (CharStringSpan_equalsNocaseP(&cmdToken, tCalOffsetP)) {
            beginJSON(reply);
            appendJSONIntValue(tCalOffsetP, EEPROMStorage_tempCalOffset(), 0, reply);
            endJSON(reply);
        } else if (CharStringSpan_equalsNocaseP(&cmdToken, PSTR("params"))) {
            beginJSON(reply);
            appendJSONIntValue(inPosP, EEPROMStorage_plungerInPos(), 0, reply);
            continueJSON(reply);
            appendJSONIntValue(outPosP, EEPROMStorage_plungerOutPos(), 0, reply);
            continueJSON(reply);
            appendJSONIntValue(posPerMlP, EEPROMStorage_posPerMl(), 0, reply);
            continueJSON(reply);
            appendJSONIntValue(mlToPumpP, EEPROMStorage_mlToPump(), 0, reply);
            endJSON(reply);
        } else if (CharStringSpan_equalsNocaseP(&cmdToken, motorPwmP)) {
            beginJSON(reply);
            appendJSONIntValue(motorPwmP, EEPROMStorage_motorPwm(), 0, reply);
            endJSON(reply);
        } else {
            validCommand = false;
        }
    } else if (CharStringSpan_equalsNocaseP(&cmdToken, PSTR("begin"))) {
        WaterPumpControl_beginPumping();
    } else if (CharStringSpan_equalsNocaseP(&cmdToken, PSTR("end"))) {
        WaterPumpControl_endPumping();
    } else if (CharStringSpan_equalsNocaseP(&cmdToken, PSTR("move"))) {
        const int16_t pos = scanIntegerToken(&cmd, &validCommand);
        if (validCommand) {
            WaterPumpControl_movePlungerTo(pos);
        }
    } else if (CharStringSpan_equalsNocaseP(&cmdToken, PSTR("stop"))) {
        WaterPumpControl_stopNow();
    } else if (CharStringSpan_equalsNocaseP(&cmdToken, PSTR("eeread"))) {
        const uint16_t eeAddr = scanIntegerToken(&cmd, &validCommand);
        if (validCommand) {
            beginJSON(reply);
            appendJSONIntValue(PSTR("EEAddr"), eeAddr, 0, reply);
            continueJSON(reply);
            appendJSONIntValue(PSTR("EEVal"), EEPROM_read((uint8_t*)eeAddr), 0, reply);
            endJSON(reply);
        }
    } else if (CharStringSpan_equalsNocaseP(&cmdToken, PSTR("eewrite"))) {
        const uint16_t eeAddr = scanIntegerToken(&cmd, &validCommand);
        if (validCommand) {
            const uint16_t eeValue = scanIntegerToken(&cmd, &validCommand);
            if (validCommand) {
                EEPROM_write((uint8_t*)eeAddr, eeValue);
            }
        }
    } else if (CharStringSpan_equalsNocaseP(&cmdToken, PSTR("ver"))) {
        Console_printLineP(swver);
    } else if (!CharStringSpan_isEmpty(&cmdToken)) {
        validCommand = false;
    }

    if (!validCommand) {
        CharString_copyP(PSTR("error"), reply);
    }

    return validCommand;
}
