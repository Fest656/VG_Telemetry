#ifndef CONFIG_H
#define CONFIG_H

// Build mode macros, do not change this

#define E2E          1 // Standard functionality Host -> Pico -> OLED
#define PICO_ECHO    2 // Testing functionality Host -> Pico -> Host
#define CONSOLE		 3 // Testing functionality Host -> CMD


#define BUILD_MODE E2E // Change this according to what you want the application to do, must be recompiled


// Do not change this
#if BUILD_MODE != CONSOLE
#define HAS_COM_PORT 1
#else
#define HAS_COM_PORT 0
#endif

#endif