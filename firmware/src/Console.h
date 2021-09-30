//
//  Console
//
//  This unit responds to characters from the serial port and
//  sends replies
//
#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>
#include <stdbool.h>
#include <avr/pgmspace.h>
#include "ConsoleInterface.h"

// sets up control pins. called once at power-up
extern void Console_Initialize (void);

// reads commands from FromUSB_Buffer and writes responses to
// ToUSB_Buffer.
// called in each iteration of the mainloop
extern void Console_task (void);

#endif  // Console_H
