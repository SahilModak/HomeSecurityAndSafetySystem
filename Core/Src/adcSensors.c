/**
 **********************************************************************************************************
 * @file           : adcSensors.c
 * @author         : Sahil Modak
 * @author         : Ali Nanji
 * @brief          : Initializes and handles peripherals that require ADC (Analog to digital conversion)
 *
 * This file contains all the methods required to initialize and handle the peripherals
 * that require ADC, which are the water sensor and the flame sensor. The public functions' prototypes
 * have been defined in adcSensor.h
 **********************************************************************************************************
 */

/** @addtogroup CORE
 * @{
 */

/** @addtogroup PERIPHERALS
 * All modules that initialize and handle the various peripherals used in this project
 * @{
 */

/** @defgroup ADC
 * Handles peripherals that require ADC (Analog to digital conversion)
 * @{
 */

/* Private includes ----------------------------------------------------------*/
#include "adcSensors.h"
#include "stm32f7xx_hal.h"
#include <stdio.h>
#define wait_delay HAL_Delay

/* Constants ------------------------------------------------------------------*/
/** @defgroup ADC_Private_Constants
 * Constants defined in adcSensors.c
 * @{
 */
const float analogReadRange = 1023.0;     // Analogs' read value ranges
const float maxVoltage = 5.0;             // Max voltage applied to analog peripherals
const float waterVoltageThreshold = 3.0;  // Water sensor's voltage threshold to indicate water has been detected
const float fireVoltageThreshold = 4.5;   // Fire sensor's voltage threshold to indicate fire has been detected
/**
 * @}
 */

/* Global Variables ------------------------------------------------------------------*/
// GPIO init structure for analog peripherals
/** @defgroup ADC_Private_Variables
 * Variables defined in adcSensors.c
 * @{
 */
GPIO_InitTypeDef GPIO_InitStruct_WaterSensor;
GPIO_InitTypeDef GPIO_InitStruct_FireSensor;
/**
 * @}
 */

/** @defgroup ADC_Private_Functions
 * Private functions defined in adcSensors.c
 * @{
 */

/**
 * @brief  Initialize GPIO for Water and Fire Sensors
 *   The water sensor uses pin A0 on the board. It is configured as follows:
 * 	           Mode = Analog
 * 	           Pull = NO PULL
 * 	           Pin = PIN 0
 *
 *   The fire sensor uses pin A1 on the board. It is configured as follows:
 * 	           Mode = Input
 * 	           Pull = NO PULL
 * 	           Pin = PIN 10
 * @param  None
 * @retval None
 */
static void MX_GPIO_Init(void){
	__HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitStruct_WaterSensor.Mode = GPIO_MODE_ANALOG; // configure to analog input mode
	GPIO_InitStruct_WaterSensor.Pin = GPIO_PIN_0;
	GPIO_InitStruct_WaterSensor.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct_WaterSensor);

	__HAL_RCC_GPIOF_CLK_ENABLE();
	GPIO_InitStruct_FireSensor.Mode = GPIO_MODE_ANALOG; // configure to analog input mode
	GPIO_InitStruct_FireSensor.Pin = GPIO_PIN_10;
	GPIO_InitStruct_FireSensor.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct_FireSensor);
}

/**
 * @brief  Configures ADC peripheral
 * @param  hadc Pointer to the ADC Handle Structure
 * @retval None
 */
static void MX_ADC_Init(ADC_HandleTypeDef *hadc)
{ 
	/* Enable ADC CLOCK */
	__HAL_RCC_ADC3_CLK_ENABLE();

	/* Configure the global features of the ADC (Clock, Resolution, Data Alignment and number
	of conversion) */		
	hadc->Instance = ADC3;
	hadc->Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
	hadc->Init.Resolution = ADC_RESOLUTION_10B;
	hadc->Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc->Init.NbrOfConversion = 1;
	hadc->Init.ScanConvMode = ENABLE;
	hadc->Init.ContinuousConvMode = ENABLE;
	hadc->Init.DiscontinuousConvMode = DISABLE;
	HAL_ADC_Init(hadc);
}

/**
 * @brief  Read ADC value on provided channel
 * Read adc value on specified channel and return converted voltage reading
 *
 * @param  hadc Pointer to the ADC Handle Structure
 * @param  channelNum Channel number at which the ADC value will be read
 * @retval Converted voltage reading
 */
float configReadADC(ADC_HandleTypeDef *hadc, uint32_t channelNum){
	float V_sense = 0;
	uint16_t adc_read;

	//configure channel
	ADC_ChannelConfTypeDef sConfig_sensor;
	sConfig_sensor.Rank = 1;
	sConfig_sensor.Channel = channelNum;//# Select the ADC Channel (ADC_CHANNEL_X)
	sConfig_sensor.SamplingTime = ADC_SAMPLETIME_28CYCLES;
	HAL_ADC_ConfigChannel(hadc, &sConfig_sensor);

	// Read analog
	HAL_ADC_Start(hadc);
	HAL_ADC_PollForConversion(hadc, HAL_MAX_DELAY);
	adc_read = HAL_ADC_GetValue(hadc); // read value
	HAL_ADC_Stop(hadc);

	// calculate the V_sense (Converted voltage reading)
	V_sense = (maxVoltage/analogReadRange) * adc_read;
	return V_sense;
}
/**
 * @}
 */

/** @defgroup ADC_Public_Functions
 * Public Functions defined in adcSensors.c
 * @{
 */

/**
 * @brief  Wrapper method to call initialization methods for ADC peripherals
 * Initialize GPIO pins and ADC
 * @param  hadc Pointer to the ADC Handle Structure
 * @retval None
 */
void initializeAdcSensors(ADC_HandleTypeDef *hadc){
	MX_GPIO_Init();
	MX_ADC_Init(hadc);
}

/**
 * @brief  Checks if any water has been detected
 * @param  hadc Pointer to the ADC Handle Structure
 * @retval True if water has been detected, false otherwise
 */
bool checkWaterDetected(ADC_HandleTypeDef *hadc){

	//return true if the converted voltage read was above 4.5v
	return (configReadADC(hadc, ADC_CHANNEL_0) > waterVoltageThreshold);
}

/**
 * @brief  Checks if any flames have been detected
 * @param  hadc Pointer to the ADC Handle Structure
 * @retval True if flames have been detected, false otherwise
 */
bool checkFireDetected(ADC_HandleTypeDef *hadc){

	//return true if the converted voltage read was below 4.5v
	return (configReadADC(hadc, ADC_CHANNEL_8) < fireVoltageThreshold);
}

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

