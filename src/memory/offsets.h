#ifndef OFFSETS_H
#define OFFSETS_H

// Lists
#define LOCAL_PLAYER  0x17E0A8

// Active Weapon
#define OFFSET_WEAPON 0x368

// Offsets from the local player
#define OFFSET_HEALTH 0xEC
#define OFFSET_ARMOR  0xF0

// Offsets that point to the values, these come from the active weapon, from the local player (must be dereferenced)
#define OFFSET_MAGAMMO 0x14
#define OFFSET_RESERVEAMMO 0x10



#endif
