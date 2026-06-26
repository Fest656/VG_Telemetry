#ifndef SERIAL_H
#define SERIAL_H

#define TEL_BUFFER 32

#include "state.h"


/*
This function is responsible for reading a single line (newline detection)
It places the line read into buffer, for embedded, getchar is better than fgets
*/
int serLineRead(char *buffer, int maxLength);

/*
This function is responsible for parsing data in a string and populating a GameState struct with it
The string should follow the formatting defined in telemetry.h
*/
int serDataHandler(GameState *statePtr, const char *buffer);

/*
This function echos back to the host program a single line, important to remember we are using a console over USB pico configuration
*/
void serLineWrite(GameState *state);

#endif