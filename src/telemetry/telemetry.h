#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <windows.h>

/*
Serial telemetry API.
Handles COM port setup and transmission of GameState data to the MCU.
All functions returning int use 1 for success, 0 for failure.
Output parameters are only written on the success path.
*/

#define TEL_BUFFER 64

// COM port configuration
#define BAUD_RATE CBR_115200
#define DATA_BITS 8

// COM port timeout configuration (all values in milliseconds)
/*
A value of MAXDWORD, combined with zero values for both the ReadTotalTimeoutConstant and ReadTotalTimeoutMultiplier members, specifies that the read operation is to return immediately with the bytes that have already been received, even if no bytes have been received.
*/
#define READ_INTERVAL_TIMEOUT_MS        MAXDWORD
#define READ_TOTAL_TIMEOUT_CONSTANT_MS  0
#define READ_TOTAL_TIMEOUT_MULT_MS      0
#define WRITE_TOTAL_TIMEOUT_CONSTANT_MS 50
#define WRITE_TOTAL_TIMEOUT_MULT_MS     10

typedef struct GameState {
    int health;
    int armor;
    int magAmmo;
    int reserveAmmo;
    int killCount;
    int deathCount;
} GameState;

// Prints the game state to stdout in a human-readable format. Used for console debugging only.
void telPrintState(GameState *state);

// Opens the specified COM port for read/write access.
// On success, writes the port handle through handlePtr. Caller must call CloseHandle on it.
int telOpenPort(const char *portName, HANDLE *handlePtr);

// Configures baud rate, data bits, parity, stop bits, and timeouts on an open COM handle.
int telSetPort(HANDLE comHandle);

// Reads incoming echoed data from the COM port and prints it to stdout.
// Used to receive data echoed from the pico
int telReadPort(HANDLE comHandle);

// Validates the data of a game state to ensure neat formatting
void telDataCheck(GameState *statePtr);

// Formats the game state as a CSV line and writes it to the COM port.
// Protocol format: "health;armor;magAmmo;reserveAmmo;killCount;deathCount\n"
int telSendState(GameState *statePtr, HANDLE comHandle);

#endif