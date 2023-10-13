/**
 **********************************************************************************************************
 * @file           : servoMotor.h
 * @author         : Sahil Modak
 * @brief          : Header file for servoMotor.c.
 *
 * This file contains the function prototypes for all public functions in servoMotor.c
 **********************************************************************************************************
 */

#include "stm32f7xx_hal.h"

// Initializations for servo motor
void initServoMotor(TIM_HandleTypeDef *htim3);

// Set servo motor angle to lock the door
void lockDoor(TIM_HandleTypeDef *htim3);

// Set servo motor angle to unlock the door
void unlockDoor(TIM_HandleTypeDef *htim3);
