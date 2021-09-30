//
// EEPROM Storage
//

#include "EEPROMStorage.h"

#include "EEPROM_Util.h"
#include "CharString.h"
#include "avr/pgmspace.h"

// This prevents the MSVC editor from tripping over EEMEM in definitions
#ifndef EEMEM
#define EEMEM
#endif

uint8_t EEMEM ee_initFlag = 1; // initialization flag. Unprogrammed EE comes up as all one's

int16_t EEMEM ee_plungerInPos;
int16_t EEMEM ee_plungerOutPos;
uint16_t EEMEM ee_posPerMl;
uint16_t EEMEM ee_mlToPump;
uint8_t EEMEM ee_motorPwm;
int16_t EEMEM ee_tempCalOffset;
uint16_t EEMEM ee_rebootInterval;   // one day

void EEPROMStorage_Initialize (void)
{
    // check if EE has been initialized
    const uint8_t initFlag = EEPROM_read((uint8_t*)&ee_initFlag);
    const uint8_t initLevel = (initFlag == 0xFF) ? 0 : initFlag;

    if (initLevel < 1) {
        // EE has not been initialized. Initialize to default settings now.

        EEPROMStorage_setPlungerInPos(50);
        EEPROMStorage_setPlungerOutPos(-50);
        EEPROMStorage_setMotorPwm(100);
        EEPROMStorage_setPosPerMl(117);
        EEPROMStorage_setMlToPump(2000);
        EEPROMStorage_setTempCalOffset(-266);
        EEPROMStorage_setRebootInterval(1440);

        // register that EEPROM is initialized
        EEPROM_write((uint8_t*)&ee_initFlag, 1);
    }
}

void EEPROMStorage_setPlungerInPos(const int16_t pos)
{
    EEPROM_writeWord((uint16_t*)&ee_plungerInPos, (uint16_t)pos);
}
int16_t EEPROMStorage_plungerInPos(void)
{
    return (int16_t)EEPROM_readWord((uint16_t*)&ee_plungerInPos);
}
void EEPROMStorage_setPlungerOutPos(const int16_t pos)
{
    EEPROM_writeWord((uint16_t*)&ee_plungerOutPos, (uint16_t)pos);
}
int16_t EEPROMStorage_plungerOutPos(void)
{
    return (int16_t)EEPROM_readWord((uint16_t*)&ee_plungerOutPos);
}

void EEPROMStorage_setPosPerMl(const uint16_t posPerMl)
{
    EEPROM_writeWord((uint16_t*)&ee_posPerMl, posPerMl);
}
uint16_t EEPROMStorage_posPerMl(void)
{
    return EEPROM_readWord((uint16_t*)&ee_posPerMl);
}

void EEPROMStorage_setMlToPump(const uint16_t mlToPump)
{
    EEPROM_writeWord((uint16_t*)&ee_mlToPump, mlToPump);
}
uint16_t EEPROMStorage_mlToPump(void)
{
    return EEPROM_readWord((uint16_t*)&ee_mlToPump);
}

void EEPROMStorage_setMotorPwm(const uint8_t pwm)
{
    EEPROM_write((uint8_t*)&ee_motorPwm, pwm);
}
uint8_t EEPROMStorage_motorPwm(void)
{
    return EEPROM_read((uint8_t*)&ee_motorPwm);
}

void EEPROMStorage_setTempCalOffset(const int16_t offset)
{
    EEPROM_writeWord((uint16_t*)&ee_tempCalOffset, (uint16_t)offset);
}

int16_t EEPROMStorage_tempCalOffset(void)
{
    return (int16_t)EEPROM_readWord((uint16_t*)&ee_tempCalOffset);
}

void EEPROMStorage_setRebootInterval(
    const uint16_t rebootMinutes)
{
    EEPROM_writeWord(&ee_rebootInterval, rebootMinutes);
}

uint16_t EEPROMStorage_rebootInterval(void)
{
    return EEPROM_readWord(&ee_rebootInterval);
}

