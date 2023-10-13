/**
 ******************************************************************************
 * @file           : commonMethods.c
 * @author         : Sahil Modak
 * @author         : Ali Nanji
 * @brief          : Defines common methods used by multiple files
 ******************************************************************************
 */

/** @addtogroup CORE
 * @{
 */

/** @defgroup Common_Methods
 * Common Methods used in various files
 * @{
 */

/* Private includes ----------------------------------------------------------*/
#include "commonMethods.h"
#include "stm32f7xx_hal.h"

/** @defgroup Common_Methods_Public_Functions
 * Public Functions defined in commonMethods.c
 * @{
 */

/**
 * @brief  Adds a delay to the program
 * @param  delay Amount of delay to add to the program
 * @retval None
 */
void wait(int delay){
	int i;
	for (i=0; i < delay; i++){}
}

/**
 * @brief  Checks if num is between min and max
 * @param  min Lower bound of range
 * @param  max Upper bound of range
 * @param  num Number to check check if its between the bounds provided
 * @retval True if num is in range, else false
 */
bool isInRange(int min, int max, int num){
	if (num >= min && num <= max) return true;
	return false;
}

/**
 * @brief  Enables GPIO Clocks required for various peripherals
 * @param None
 * @retval None
 */
void enableClocks(void){
	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOI_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
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
