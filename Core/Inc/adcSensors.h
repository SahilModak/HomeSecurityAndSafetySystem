/**
 **********************************************************************************************************
 * @file           : adcSensors.h
 * @author         : Sahil Modak
 * @author         : Ali Nanji
 * @brief          : Header file for adcSensors.c
 *
 * This file contains the function prototypes for all public functions in adcSensor.c
 **********************************************************************************************************
 */

#include <stdbool.h>
#include "stm32f7xx_hal.h"

// Call initialization methods for adc peripherals
void initializeAdcSensors(ADC_HandleTypeDef *hadc);

// Check if any water has been detected
bool checkWaterDetected(ADC_HandleTypeDef *hadc_waterSensor);

// Check if any fire has been deteted
bool checkFireDetected(ADC_HandleTypeDef *hadc_fireSensor);
