//
// EEPROM Storage
//
// Storage of non-volatile settings and data
//

#ifndef EEPROMSTORAGE_H
#define EEPROMSTORAGE_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>

extern void EEPROMStorage_Initialize (void);

// console echo state
extern void EEPROMStorage_setEcho(const bool echo);
extern bool EEPROMStorage_echo();

// units are odometer counts
extern void EEPROMStorage_setPlungerInPos(const int16_t pos);
extern int16_t EEPROMStorage_plungerInPos(void);
extern void EEPROMStorage_setPlungerOutPos(const int16_t pos);
extern int16_t EEPROMStorage_plungerOutPos(void);

// ratio of odometer counts to ml
extern void EEPROMStorage_setPosPerMl(const uint16_t posPerMl);
extern uint16_t EEPROMStorage_posPerMl(void);

// volume of water to pump when tank is full (units: ml)
extern void EEPROMStorage_setMlToPump(const uint16_t mlToPump);
extern uint16_t EEPROMStorage_mlToPump(void);

// pwm: 0..255, which is 0 to  100% duty cycle
extern void EEPROMStorage_setMotorPwm(const uint8_t pwm);
extern uint8_t EEPROMStorage_motorPwm(void);

// internal temperature sensor calibration offset
extern void EEPROMStorage_setTempCalOffset(const int16_t offset);
extern int16_t EEPROMStorage_tempCalOffset(void);

// how long to go since last reboot. units are minutes
extern void EEPROMStorage_setRebootInterval(
    const uint16_t rebootMinutes);
extern uint16_t EEPROMStorage_rebootInterval(void);

#endif		// EEPROMSTORAGE