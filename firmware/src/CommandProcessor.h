//
// Command processor
//
// Interprets and executes commands from the console
//

#ifndef COMMANDPROCESSOR_H
#define COMMANDPROCESSOR_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include "CharStringSpan.h"

// buffer that clients can use to accumulate command characters
extern CharString_t CommandProcessor_incomingCommand;

// output from commands is put into this string
extern CharString_t CommandProcessor_commandReply;

extern bool CommandProcessor_executeCommand (
    const CharStringSpan_t* command,
    CharString_t* reply);

#endif  // COMMANDPROCESSOR_H