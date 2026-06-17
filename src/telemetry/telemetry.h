#ifndef TELEMETRY_H
#define TELEMETRY_H

// COM port configuration
#define BAUD_RATE CBR_115200
#define DATA_BITS 8


#include <windows.h>

/*
This file is responsible for the funtions related to the production and transfer of telemetry data from the game to the host application and then the MCU.
*/

typedef struct GameState {
	int health;
	int armor;
	int magAmmo;
	int reserveAmmo;
} GameState;

// This function is responsible for applying the correct ASCII format to any game state, only used for the console for debugging purposed
void telStateFormat(GameState* state);

// This function returns a handle to the desired COM port using createFile
HANDLE telOpenPort(const char *portName);

// This function is called to set the desired control settings of a serial communications device. This is done through the DCB struct
int telSetPort( HANDLE comHandle);

// Calls the formatting function on the game state and sends it over the serial connection
int telSendState(GameState *state, HANDLE comHandle);


#endif