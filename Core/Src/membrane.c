/**
 ******************************************************************************
 * @file           : membrane.c
 * @author         : Sahil Modak
 * @brief          : Initializes and handles membrane keypad
 *
 * This file contains all the methods required to initialize and handle the
 * membrane keypad. The public functions' prototypes have been defined in
 * membrane.h
 ******************************************************************************
 */

/** @addtogroup CORE
 * @{
 */

/** @addtogroup PERIPHERALS
 * @{
 */

/** @defgroup MEMBRANE_KEY_PAD
 * Handles Membrane Key Pad
 * @{
 */

/* Private includes ----------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_gpio.h"
#include "commonMethods.h"
#include "membrane.h"

/* Constants ------------------------------------------------------------------*/
/** @defgroup MEMBRANE_KEY_PAD_Private_Constants
 * Constants defined in membrane.c
 * @{
 */
const int currMemPin = 1234;
/**
 * @}
 */

/* Global Variables ------------------------------------------------------------------*/
/** @defgroup MEMBRANE_KEY_PAD_Private_Variables
 * Variables defined in membrane.c
 * @{
 */
GPIO_InitTypeDef gpio8;
GPIO_InitTypeDef gpio9;
GPIO_InitTypeDef gpio10;
GPIO_InitTypeDef gpio11;
GPIO_InitTypeDef gpio12;
GPIO_InitTypeDef gpio13;
GPIO_InitTypeDef gpio14;
GPIO_InitTypeDef gpio15;
/**
 * @}
 */

/* Private define ------------------------------------------------------------*/
/** @defgroup MEMBRANE_KEY_PAD_Structs
 * Structs defined in membrane.c
 * @{
 */

/*
 * @brief Private struct defined to store details of a pin used for the membrane keypad
 * */
struct pin {
	GPIO_InitTypeDef* gpio; /**< GPIO Init structure definition  */
	GPIO_TypeDef* GPIOx;    /**< GPIO Type */
} pin;

/*
 * @brief Private struct defined to store details of all pins used for the membrane keypad
 * */
static struct pin allPins[8];
/**
 * @}
 */

/** @defgroup  MEMBRANE_KEY_PAD_Private_Functions
 * Private Functions defined in membrane.c
 * @{
 */

/**
 * @brief  Resets Membrane pins to give value
 * @param  start Start index of list to reset
 * @param  stop Last index of  list to reset
 * @param  value New pin state for pins
 * @retval None
 */
void resetMembranePins(int start, int stop, GPIO_PinState value){
	int i;
	for (i = start; i < stop; i++){
		HAL_GPIO_WritePin(allPins[i].GPIOx, allPins[i].gpio->Pin, value);
	}
}

/**
 * @brief  Initializes membrane keybad pins, to input or output
 * @param  gpio Pointer to the pin structure
 * @param  GPIOx Pointer to the gpio type
 * @param  GPIO_Pin Pin number
 * @Param  pos Position of pin in the allPins array
 * @retval None
 */
void initializeMembrane(GPIO_InitTypeDef *gpio, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint16_t pos){
	if (pos < 4){
		// set mode as input for membrane columns
		gpio->Mode = GPIO_MODE_INPUT;
	}
	else{
		// set mode as output for all other pins
		gpio->Mode = GPIO_MODE_OUTPUT_PP;
	}
	gpio->Pull = GPIO_PULLDOWN;
	gpio->Speed = GPIO_SPEED_HIGH;
	gpio->Pin = GPIO_Pin;

	HAL_GPIO_Init(GPIOx, gpio);

	allPins[pos].gpio = gpio;
	allPins[pos].GPIOx = GPIOx;
}

/**
 * @brief  calculates number pressed depending on membrane keypad input
 * @param  row Row number of button pressed
 * @param  col Column number of button pressed
 * @retval number pressed on the membrane keypad
 */
int getMembraneNum(int col, int row){
	row = row - 4;
	if (row == 0 && col == 2){
		return 0;
	}
	if (row == 0 && col == 3){
		return -3;
	}
	if (row == 0 && col == 1){
		return -2;
	}
	if (row == 0 || col == 0){ 
		return -1;
	}
	col = 4 - col;
	row = 3 - row;
	return row * 3 + col;
}

/**
 * @brief  Checks which button on the membrane keypad is pressed
 * @param  None
 * @retval Value of the button pressed on the membrane keypad
 */
int readMembrane(){
	int row, col, num;
	for (row = 4; row < 8; row++){
		resetMembranePins(0, 4, GPIO_PIN_RESET);
		resetMembranePins(row, row + 1, GPIO_PIN_SET);
		for (col = 0; col < 4; col++){
			if(HAL_GPIO_ReadPin(allPins[col].GPIOx, allPins[col].gpio->Pin) == GPIO_PIN_SET){
				resetMembranePins(0, 4, GPIO_PIN_RESET);
				num = getMembraneNum(col, row);
				wait(100000);
				while(HAL_GPIO_ReadPin(allPins[col].GPIOx, allPins[col].gpio->Pin) == GPIO_PIN_SET){
					num = num;
				}
				resetMembranePins(4, 8, GPIO_PIN_RESET);
				return num;
			}
		}
		resetMembranePins(row, row + 1, GPIO_PIN_RESET);
	}
	resetMembranePins(0, 8, GPIO_PIN_RESET);
	return -1;
}

/**
 * @}
 */

/** @defgroup  MEMBRANE_KEY_PAD_Public_Functions
 * Public Functions defined in membrane.c
 * @{
 */

/**
 * @brief  Initializes all pins that will be used by the membrane keypad
 * 			The membrane keypad uses pin D8 - D15 on the board. It is configured as follows:
 * 	           Mode = Input/Output
 * 	           Pull = PULL DOWN
 * 	           Speed = SPEED HIGH
 * @param  None
 * @retval None
 */
void initializeMembranePins (void){

	// membrane pins
	initializeMembrane(&gpio8, GPIOI, GPIO_PIN_2, 0);
	initializeMembrane(&gpio9, GPIOA, GPIO_PIN_15, 1);
	initializeMembrane(&gpio10, GPIOA, GPIO_PIN_8, 2);
	initializeMembrane(&gpio11, GPIOB, GPIO_PIN_15, 3);
	initializeMembrane(&gpio12, GPIOB, GPIO_PIN_14, 4);
	initializeMembrane(&gpio13, GPIOI, GPIO_PIN_1, 5);
	initializeMembrane(&gpio14, GPIOB, GPIO_PIN_9, 6);	
	initializeMembrane(&gpio15, GPIOB, GPIO_PIN_8, 7);
}

/**
 * @brief  Checks if the PIN entered on the keypad is correct
 * @param  currentMemDigitCount Pointer to the count of digits currently entered
 * @param  currentMemPin Current value of Pin entered
 * @param  membranePinLen Length of PIN to be verified
 * @retval True if PIN is correct, False otherwise
 */
bool checkMemPin(int *currentMemDigitCount, int *currentMemPin, int membranePinLen){
	int currDigit = readMembrane();
	int x = currMemPin;
	if (currDigit == -2){
		bool y = *currentMemPin == x;
		*currentMemDigitCount = 0;
		*currentMemPin = 0;
		return y;
	}
	else if (currDigit == -3){
		*currentMemDigitCount = 0;
		*currentMemPin = 0;
	}
	else if (currDigit != -1){
		if(*currentMemDigitCount < membranePinLen){
			*currentMemDigitCount = *currentMemDigitCount + 1;
			*currentMemPin = *currentMemPin * 10 + currDigit;
		}
		else{
			*currentMemDigitCount = 0;
			*currentMemPin = 0;
		}
	}
	return false;
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

