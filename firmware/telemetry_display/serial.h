#ifndef SERIAL_H
#define SERIAL_H

#include "state.h"

// Serial read timeout for getchar_timeout_us (in microseconds)
#define SERIAL_CHAR_TIMEOUT_US 100

// Maximum line buffer size for incoming telemetry packets
#define TEL_BUFFER 64

// Reads one newline-terminated line from USB serial into buffer.
// Returns 1 if a complete line was read, 0 on timeout or buffer full (no complete line available).
int serLineRead(char *buffer, int maxLength);

// Parses a CSV telemetry line (format: "health;armor;magAmmo;reserveAmmo;killCount;deathCount") into statePtr. 
// Returns 1 if all 6 fields were parsed successfully, 0 on malformed input.
int serDataHandler(GameState *statePtr, const char *buffer);

// Echoes the parsed game state back to the host over USB serial for debugging.
void serLineWrite(GameState *statePtr);

#endif