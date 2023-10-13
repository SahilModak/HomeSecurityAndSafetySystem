/**
 ******************************************************************************
 * @file           : pinpad.c
 * @author         : Ali Nanji
 * @brief          : Handles Pin Pad page of the UI
 *
 * This file contains all the methods required to manage the Pin Pad page of the
 * UI. The pin pad page displays a pin pad where if the user enters the correct
 * pin, they are directed to the status page. The public functions' prototypes
 * have been defined in pinpad.h
 ******************************************************************************
 */

/** @addtogroup CORE
 * @{
 */

/** @addtogroup DISPLAY
 * Contains modules that handle the various UI pages
 * @{
 */

/** @defgroup PIN_PAD_PAGE
 * Handle Pin Pad page on the UI
 * @{
 */

/* Private includes ----------------------------------------------------------*/
#include "pinpad.h"
#include <stdio.h>
#include "stm32f7xx_hal.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"
#include "commonMethods.h"
#define wait_delay HAL_Delay

#ifdef __RTX
extern uint32_t os_time;
uint32_t HAL_GetTick(void) {
	return os_time;
}
#endif

/* Constants ------------------------------------------------------------------*/
/** @defgroup PIN_PAD_PAGE_Private_Constants
 * Constants defined in pinpad.c
 * @{
 */
const int dispDigitsLimit = 4;                                // Screen pinpad only accepts 4 digits
const int numPadBoxWidth = 157;                               // Width of each number's box
const int numPadBoxHeight = 38;                               // Height of each number's box
const int correctPassword[4] = { 0, 0, 0, 0};                 // Current correct password
const int numPadCoordinates[10][3] = { { 160, 231, 0 },       // Coordinates for each of the numbers on the pinpad {x, y, num}
		{ 1, 111, 1 }, { 160, 111, 2 }, { 319, 111, 3 },
		{ 1, 151, 4 }, { 160, 151, 5 }, { 319, 151, 6 },
		{ 1, 191, 7 }, { 160, 191, 8 }, { 319, 191, 9 }
};
/**
 * @}
 */

/* Global Variables ------------------------------------------------------------------*/
/** @defgroup PIN_PAD_PAGE_Private_Variables
 * Variables defined in pinpad.c
 * @{
 */
TS_StateTypeDef tsc_state;
/**
 * @}
 */

/** @defgroup PIN_PAD_PAGE_Private_Functions
 * Private functions defined in pinpad.c
 * @{
 */

/**
 * @brief  Draw a rectangle for a key on the pin pad
 * @param  x Top left x position
 * @param  y Top Left y position
 * @param  digit Number to display inside rectagle
 * @retval None
 */
void drawDigitBox(int x, int y, int digit){
	char buffer[1];
	sprintf(buffer, "%i", digit);

	BSP_LCD_SetTextColor(LCD_COLOR_LIGHTGRAY);
	BSP_LCD_DrawRect(x, y, numPadBoxWidth, numPadBoxHeight);

	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_DisplayStringAt(x + 70, y + 10, buffer, LEFT_MODE);

	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
}

/**
 * @brief  Check the coordinate of the user's touch to see what key has been pressed
 * @param  x X position of the touch
 * @param  y Y position of the touch
 * @retval Key touched
 */
int checkCoords(int x, int y){
	int i;
	if (y < 111) return -1;

	for (i=0; i < 10; i++){
		int currX = numPadCoordinates[i][0];
		int currY = numPadCoordinates[i][1];
		if (isInRange(currX, currX + numPadBoxWidth, x) && isInRange(currY, currY + numPadBoxHeight, y)){
			return numPadCoordinates[i][2];
		}
	}

	if (isInRange(319, 319 + numPadBoxWidth, x) && isInRange(231, 231 + numPadBoxHeight, y)) return -2;

	return -1;
}

/**
 * @brief  Draw the entire num pad
 * @param  None
 * @retval None
 */
void drawNumPad(void){
	int i;

	for (i=0; i < 10; i++){
		drawDigitBox(numPadCoordinates[i][0], numPadCoordinates[i][1], i);
	}

	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	BSP_LCD_SetTextColor(LCD_COLOR_LIGHTGRAY);
	BSP_LCD_DrawRect(1, 231, 157, 38);
	BSP_LCD_DrawRect(319, 231, 157, 38);

	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_DisplayStringAt(319 + 55, 231 + 10, (uint8_t *)"Del", LEFT_MODE);
}

/**
 * @brief  Draw dashes to display current pin
 * @param  None
 * @retval None
 */
void dispDigitSpaces(void){
	BSP_LCD_SetTextColor(LCD_COLOR_DARKGRAY);
	BSP_LCD_DrawHLine(131, 80, 20);
	BSP_LCD_DrawHLine(197, 80, 20);
	BSP_LCD_DrawHLine(263, 80, 20);
	BSP_LCD_DrawHLine(329, 80, 20);
}

/**
 * @brief  Display the digit the user touched on
 * @param  num Number to display
 * @param  currentDispDigitCount Position to display number at
 * @retval None
 */
void dispDigit(int num, int currentDispDigitCount){
	char buffer[1];
	sprintf(buffer, "%i", num);

	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	switch (currentDispDigitCount)
	{
	case 1:
		BSP_LCD_DisplayStringAt(133, 55, buffer, LEFT_MODE);
		break;
	case 2:
		BSP_LCD_DisplayStringAt(199, 55, buffer, LEFT_MODE);
		break;
	case 3:
		BSP_LCD_DisplayStringAt(265, 55, buffer, LEFT_MODE);
		break;
	case 4:
		BSP_LCD_DisplayStringAt(331, 55, buffer, LEFT_MODE);
		break;
	}
}

/**
 * @brief  Delete one digit from current pin
 * @param  currentDispDigitCount Position to delete number from
 * @retval None
 */
void deleteDigit(int currentDispDigitCount){
	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	switch (currentDispDigitCount)
	{
	case 1:
		BSP_LCD_DisplayStringAt(133, 55, (uint8_t *)" ", LEFT_MODE);
		break;
	case 2:
		BSP_LCD_DisplayStringAt(199, 55, (uint8_t *)" ", LEFT_MODE);
		break;
	case 3:
		BSP_LCD_DisplayStringAt(265, 55, (uint8_t *)" ", LEFT_MODE);
		break;
	case 4:
		BSP_LCD_DisplayStringAt(331, 55, (uint8_t *)" ", LEFT_MODE);
		break;
	}
}

/**
 * @brief  Delete all digits from current pin
 * @param  None
 * @retval None
 */
void clearDigits(void){
	int i;
	for (i=dispDigitsLimit; i>=0; i--){
		deleteDigit(i);
	}
}

/**
 * @brief  Clear any banner
 * @param  None
 * @retval None
 */
void clearText(void){
	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_DisplayStringAt(90, 10, (uint8_t *)"                    ", LEFT_MODE);
}

/**
 * @brief  Display "Wrong Password" banner
 * @param  None
 * @retval None
 */
void dispWrongPassword(void){
	clearText();
	BSP_LCD_SetBackColor(LCD_COLOR_RED);
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_DisplayStringAt(114, 10, (uint8_t *)" Wrong Password ", LEFT_MODE);
}

/**
 * @brief  Display "Correct Password" banner
 * @param  None
 * @retval None
 */
void dispCorrectPassword(void){
	clearText();
	BSP_LCD_SetBackColor(LCD_COLOR_GREEN);
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_DisplayStringAt(90, 10, (uint8_t *)" Correct Password ", LEFT_MODE);
}

/**
 * @brief  Check if the currently input pin is correct
 * @param  currentDispDigits Array of currently input pin digits
 * @param  currentDispDigitCount Pointer to current pin length count
 * @retval True if password is correct, false otherwise
 */
bool checkPassword(int currentDispDigits[], int *currentDispDigitCount){
	int i;
	for (i=0; i<dispDigitsLimit; i++){
		if (currentDispDigits[i] != correctPassword[i]){
			dispWrongPassword();
			wait(1000000);
			clearDigits();
			*currentDispDigitCount = 0;
			return false;
		}
	}
	dispCorrectPassword();
	wait(1000000);
	clearDigits();
	*currentDispDigitCount = 0;
	return true;
}

/**
 * @}
 */


/** @defgroup PIN_PAD_PAGE_Public_Functions
 * Public functions defined in pinpad.c
 * @{
 */

/**
 * @brief  Draw the entire lock screen
 * @param  None
 * @retval None
 */
void displayLockScreen(void){
	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	BSP_LCD_Clear(LCD_COLOR_WHITE);

	// Drawing lock screen
	drawNumPad();
	dispDigitSpaces();
}

/**
 * @brief  Initialize everything needed for LCD and touch
 * @param  None
 * @retval None
 */
void initializeDisplay(void){
	/* Enable the CRC Module */
	__HAL_RCC_CRC_CLK_ENABLE();

	/* Initialize Touch screen */
	BSP_TS_Init(480, 272);

	/* Initialize GLCD */
	BSP_LCD_Init();
	BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);
	BSP_LCD_DisplayOn();

	BSP_LCD_SelectLayer(0);
	BSP_LCD_Clear(LCD_COLOR_WHITE);

	/* Draw initial lock screen */
	displayLockScreen();
}

/**
 * @brief  Wrapper method called from main loop to check current pin
 *
 * This method contains all the main logic for the Pin Pad page. It checks
 * if the user has currently tapped any key and does corresponding action.
 * The method also remains in a loop until the user stops touching the screen
 * to prevent unintentional touches.
 *
 * @param  currentDispDigits Array of currently input pin digits
 * @param  currentDispDigitCount Pointer to current pin length count
 * @retval None
 */
bool checkPin(int *currentDispDigitCount, int *currentDispDigits) {
	bool retVal = false;
	int check;

	BSP_TS_GetState(&tsc_state);
	if (tsc_state.touchDetected){

		// Check what number has been touched if any
		check = checkCoords(tsc_state.touchX[0], tsc_state.touchY[0]);
		if (check >= 0){

			// Clear any banners from previous loop
			clearText();

			// If input has not exceeded pin limit, display input number and add it to current password array
			if (*currentDispDigitCount < dispDigitsLimit){
				*currentDispDigitCount = *currentDispDigitCount + 1;
				currentDispDigits[*currentDispDigitCount-1] = check;
				dispDigit(check, *currentDispDigitCount);

				// If after the current touch, pin has reached the pin limit, check to see if the current pin is correct
				if (*currentDispDigitCount == dispDigitsLimit){
					retVal = checkPassword(currentDispDigits, currentDispDigitCount);
				}
			}
		}
		// If user clicks on delete, remove one digit from the currently input pin
		if (check == -2 && *currentDispDigitCount > 0){
			clearText();
			deleteDigit(*currentDispDigitCount);
			*currentDispDigitCount = *currentDispDigitCount - 1;
		}

		// Keep looping until user stops touching the screen to prevent accidental taps
		for(;;){
			BSP_TS_GetState(&tsc_state);
			if(!tsc_state.touchDetected) break;
		}
	}
	return retVal;
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
