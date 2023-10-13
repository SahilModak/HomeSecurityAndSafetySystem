/**
 **********************************************************************************************************
 * @file           : pinpad.h
 * @author         : Ali Nanji
 * @brief          : Header file for pinpad.c.
 *
 * This file contains the function prototypes for all public functions in pinpad.c
 **********************************************************************************************************
 */

#include <stdbool.h>

// Call all initialization methods for touch and display
void initializeDisplay(void);

// Display lockscreen with pinpad
void displayLockScreen(void);

// Check if the user has touched the screen, and if they have, is the currently inputted value the correct pin
// Returns true of the pin is correct and false otherwise
bool checkPin(int *currentDispDigitCount, int *currentDispDigits);
