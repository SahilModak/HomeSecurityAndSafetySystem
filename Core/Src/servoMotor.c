/**
 ******************************************************************************
 * @file           : servoMotor.c
 * @author         : Sahil Modak
 * @brief          : Initializes and handles servo motor
 *
 * This file contains all the methods required to lock and unlock the front
 * door using the servo motor. The public functions' prototypes have been
 * defined in servoMotor.h
 ******************************************************************************
 */

/** @addtogroup CORE
 * @{
 */

/** @addtogroup PERIPHERALS
 * @{
 */

/** @defgroup SERVO_MOTOR
 * Handles Servo Motor
 * @{
 */

/* Private includes ----------------------------------------------------------*/
#include "servoMotor.h"
#include "stm32f7xx_hal.h"

/** @defgroup SERVO_MOTOR_Private_Functions
 * Private Functions defined in servoMotor.c
 * @{
 */

/**
 * @brief Servo Motor Pin PB4 Initialization Function
 * 	  The Servo Motor uses pin D3 on the board. It is configured as follows:
 * 	      Mode = Alternate Function Push Pull
 * 	      Pull = No Pull
 * 	      Speed = SPEED HIGH
 * 	      Pin = PIN 4
 * 	      Alternate = TIM3 Alternate Function
 *
 * @param None
 * @retval None
 */
void PB4_Init(void){
	GPIO_InitTypeDef gpio;

	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	gpio.Pin = GPIO_PIN_4;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;
	gpio.Alternate = GPIO_AF2_TIM3;
	HAL_GPIO_Init(GPIOB, &gpio);
}

/**
 * @brief  Timer (TIM3) and Channel (CH1) initialization.
 *      TIM3 is configured as follows:
 *           Prescaler = 89999
 *           Counter Mode = Up
 *           Period = 81
 *           Clock Division = 1
 *           Auto Reload Preload = DISABLE
 *
 *           ClockSource = TIM_CLOCKSOURCE_INTERNAL
 *
 * @param  htim3 Pointer to TIM3 Handle Structure
 * @retval None
 */
static void TIM3_Init(TIM_HandleTypeDef *htim3){
	TIM_ClockConfigTypeDef sClockSourceConfig;
	TIM_OC_InitTypeDef sConfigOC;

	//Timer configuration
	htim3->Instance = TIM3;
	htim3->Init.Prescaler = 90000-1;
	htim3->Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3->Init.Period = 81;
	htim3->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	HAL_TIM_Base_Init(htim3);

	//Set the timer in PWM mode
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(htim3, &sClockSourceConfig);
	HAL_TIM_PWM_Init(htim3);

	//Channel configuration
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_PWM_ConfigChannel(htim3, &sConfigOC, TIM_CHANNEL_1);

	PB4_Init();
}

/**
 * @}
 */

/** @defgroup SERVO_MOTOR_Public_Functions
 * Public Functions defined in servoMotor.c
 * @{
 */

/**
 * @brief  Wrapper method for initializations of the servo motor
 * @param  htim3 Pointer to TIM3 Handle Structure
 * @retval None
 */
void initServoMotor(TIM_HandleTypeDef *htim3){
	//Enable peripheral clock for TIM2
	__HAL_RCC_TIM3_CLK_ENABLE();

	//Initialize TIM3, CH1 and PB4
	TIM3_Init(htim3);

	//Start PWM on TIM3_CH1
	HAL_TIM_PWM_Start(htim3, TIM_CHANNEL_1);
	htim3->Instance->CCR1 = 5;
}

/**
 * @brief  Moves servo motor arm to 90 Degree position to lock the door
 * @param  htim3 Pointer to TIM3 Handle Structure
 * @retval None
 */
void lockDoor(TIM_HandleTypeDef *htim3){
	htim3->Instance->CCR1 = 5;
}

/**
 * @brief  Moves servo motor arm to 180 Degree position to unlock the door
 * @param  htim3 Pointer to TIM3 Handle Structure
 * @retval None
 */
void unlockDoor(TIM_HandleTypeDef *htim3){
	htim3->Instance->CCR1 = 9;
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
