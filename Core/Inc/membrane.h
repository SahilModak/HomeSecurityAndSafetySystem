/**
 **********************************************************************************************************
 * @file           : membrane.h
 * @author         : Sahil Modak
 * @brief          : Header file for membrane.c.
 *
 * This file contains the function prototypes for all public functions in membrane.c
 **********************************************************************************************************
 */

#include <stdbool.h>

void initializeMembranePins (void);
bool checkMemPin(int *currentMemDigitCount, int *currentMemPin, int membranePinLen);
