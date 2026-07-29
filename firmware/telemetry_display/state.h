#ifndef STATE_H
#define STATE_H

// Telemetry data received from the host. Mirrors the GameState defined on the host side.

typedef struct GameState {
	int health;
	int armor;
	int magAmmo;
	int reserveAmmo;
	int killCount;
	int deathCount;
} GameState;

#endif