//
//  Console interface
//
//  How it works:
//     Collects incoming characters from the UART until a cr is received
//     and then passes the string to the command processor.
//     Puts message strings out to the UART
//
//  I/O Pin assignments
//
#include "Console.h"

#include "SystemTime.h"
#include "CommandProcessor.h"
#include "EEPROMStorage.h"
#include "UART_async.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "MSVS_AVR.h"

#define ANSI_ESCAPE_SEQUENCE(EscapeSeq)  "\33[" EscapeSeq
#define ESC_CURSOR_POS(Line, Column)    ANSI_ESCAPE_SEQUENCE(#Line ";" #Column "H")
#define ESC_ERASE_LINE                  ANSI_ESCAPE_SEQUENCE("K")
#define ESC_CURSOR_POS_RESTORE          ANSI_ESCAPE_SEQUENCE("u")

const char PROGMEM crP[] = { 13,0 };
const char PROGMEM crlfP[] = { 13,10,0 };

ByteQueue_define(16, rxQueue, static);
ByteQueue_define(80, txQueue, static);

void Console_Initialize (void)
{
    UART_init(true, &rxQueue, &txQueue);
    UART_set_baud_rate(4800);
}

void Console_task (void)
{
    char cmdByte;
    if (UART_read_byte(&cmdByte)) {
        switch (cmdByte) {
            case '\r' : {
                // command complete. execute it
                Console_printP(crlfP);
                CharStringSpan_t command;
                CharStringSpan_init(&CommandProcessor_incomingCommand, &command);
                CommandProcessor_executeCommand(&command, &CommandProcessor_commandReply);
                if (!CharString_isEmpty(&CommandProcessor_commandReply)) {
                    Console_printLineCS(&CommandProcessor_commandReply);
                    CharString_clear(&CommandProcessor_commandReply);
                }
                CharString_clear(&CommandProcessor_incomingCommand);
                }
                break;
            case 0x7f :
                // delete last char
                CharString_truncate(CharString_length(&CommandProcessor_incomingCommand) - 1,
                    &CommandProcessor_incomingCommand);
                break;
            default : 
                // command not complete yet. append to command buffer
                CharString_appendC(cmdByte, &CommandProcessor_incomingCommand);
                break;
        }
        // echo current command
        Console_printP(crP);
        Console_printCS(&CommandProcessor_incomingCommand);
        Console_print(ESC_ERASE_LINE);
    }
}

void Console_print (
	const char* text)
{
    // not checking if write was successful.
    // will need to revisit this. Maybe console has
    // a buffer to hold text that didn't get written.
    // Would need a console task to push it out.
    UART_write_string(text);
}

void Console_printLine (
	const char* text)
{
    Console_print(text);
    Console_printNewline();
}

void Console_printCS (
    const CharString_t* text)
{
    UART_write_stringCS(text);
}

void Console_printLineCS (
	const CharString_t* text)
{
    Console_printCS(text);
    Console_printNewline();
}

void Console_printNewline (void)
{
    Console_printP(PSTR("\r\n"));
}

void Console_printP (
	PGM_P text)
{
    // not checking if write was successful.
    // will need to revisit this. Maybe console has
    // a buffer to hold text that didn't get written.
    // Would need a console task to push it out.
    UART_write_stringP(text);
}

void Console_printLineP (
	PGM_P text)
{
    Console_printP(text);
    Console_printNewline();
}
