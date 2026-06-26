#include <stdio.h>
#include <windows.h>
#include "telemetry.h"

// https://learn.microsoft.com/en-us/previous-versions/ms810467(v=msdn.10)?redirectedfrom=MSDN

// The protocol transports data in the following format: [ Health; Armour; Mag; Reserve ]
void telStateFormat(GameState *state) {
    printf("HP:%d\nArmor:%d\nAmmo in mag:%d\nReserve ammo:%d\n", state->health, state->armor, state->magAmmo, state->reserveAmmo);
}

/*
https://learn.microsoft.com/en-us/windows/win32/devio/configuring-a-communications-resource
https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea
*/
HANDLE telOpenPort(const char *portName) {
    HANDLE comHandle = CreateFile(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (comHandle == INVALID_HANDLE_VALUE) {
        printf("Error: Create file did not obtain a handle to COM1.\n");
        return INVALID_HANDLE_VALUE;
    }
    return comHandle;
}

/*
https://learn.microsoft.com/en-us/windows/win32/api/winbase/ns-winbase-dcb
https://learn.microsoft.com/en-us/windows/win32/devio/configuring-a-communications-resource
https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/aa366877(v=vs.85) - SecureZeroMemory
https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-setcommstate
https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-getcommstate
*/
int telSetPort(HANDLE comHandle) {
    BOOL configState;
    // Initialize the timeout struct zeroed out in all its memory
    COMMTIMEOUTS comTimeOut;
    SecureZeroMemory(&comTimeOut, sizeof(COMMTIMEOUTS));
    // In milliseconds
    comTimeOut.ReadIntervalTimeout = 50;
    comTimeOut.ReadTotalTimeoutConstant = 50;
    comTimeOut.ReadTotalTimeoutMultiplier = 10;
    comTimeOut.WriteTotalTimeoutConstant = 50;
    comTimeOut.WriteTotalTimeoutMultiplier = 10;
    // Initialize the DCB struct zeroed out in all its memory
    DCB dcb;
    DWORD dcbLen = sizeof(DCB);
    SecureZeroMemory(&dcb, dcbLen);

    configState = GetCommState(comHandle, &dcb);
    if (configState == FALSE) {
        printf("Error: Failed to get current COM configuration");
        return 0;
    }

    // Configure the DCB struct
    dcb.DCBlength = dcbLen;
    dcb.BaudRate = BAUD_RATE;     
    dcb.ByteSize = DATA_BITS;             
    dcb.Parity   = NOPARITY;      
    dcb.StopBits = ONESTOPBIT;    
    
    configState = SetCommState(comHandle, &dcb);
    if (configState == FALSE) {
        printf("Error: Failed to set COM configuration");
        return 0;
    }

    configState = SetCommTimeouts(comHandle, &comTimeOut);
    if (configState == FALSE) {
        printf("Error: Failed to set COM timeout configuration");
        return 0;
    }

    configState = GetCommState(comHandle, &dcb);
    if (configState == FALSE) {
        printf("Error: GetCommState failed after setting our configuration");
        return 0;
    }

    return 1;
}

/*
https://learn.microsoft.com/pt-br/cpp/c-runtime-library/reference/snprintf-snprintf-snprintf-l-snwprintf-snwprintf-l?view=msvc-170
https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile
*/
int telSendState(GameState *state, HANDLE comHandle) {
    char buffer[64];
    
    int bytesToWrite = snprintf(buffer, sizeof(buffer), "%d;%d;%d;%d\n", state->health, state->armor, state->reserveAmmo, state->magAmmo);
                                
    DWORD bytesWritten;
    BOOL writeStatus = WriteFile(comHandle, buffer, bytesToWrite, &bytesWritten, NULL);
    
    if (writeStatus == FALSE) {
        printf("Error: Failed to write to COM port\n");
        return 0;
    }
    
    return 1; 
}
