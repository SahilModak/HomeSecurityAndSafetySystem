/**
 **********************************************************************************************************
 * @file           : commonMethods.h
 * @author         : Sahil Modak
 * @author         : Ali Nanji
 * @brief          : Header file for commonMethods.c.
 *
 * This file contains the function prototypes for all public functions in commonMethods.c
 **********************************************************************************************************
 */

#include <stdbool.h>

// Adds a delay to the program
void wait(int delay);

// Checks if num is between min and max
bool isInRange(int min, int max, int num);

// Enable gpio clocks needed for perpherals
void enableClocks(void);
