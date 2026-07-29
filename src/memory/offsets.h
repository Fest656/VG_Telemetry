#ifndef OFFSETS_H
#define OFFSETS_H

// Lists
#define LOCAL_PLAYER  0x17E0A8

// Offsets from the local player (must be dereferenced)
#define OFFSET_HEALTH 0xEC
#define OFFSET_ARMOR  0xF0
#define OFFSET_KILLS  0x1DC
#define OFFSET_DEATHS 0x1E4

// Active Weapon, from the local player (must be dereferenced)
#define OFFSET_WEAPON 0x368


// Offsets from the active weapon (must be dereferenced)
#define OFFSET_MAGAMMO 0x14
#define OFFSET_RESERVEAMMO 0x10

#endif
