/**
 ******************************************************************************
 * @file           : statusPage.c
 * @author         : Ali Nanji
 * @brief          : Handles Status page of the UI
 *
 * This file contains all the methods required to manage the status page of the
 * UI. The status page displays all the warnings related to each of the
 * peripherals. The public functions' prototypes have been defined in
 * statusPage.h
 ******************************************************************************
 */

/** @addtogroup CORE
 * @{
 */

/** @addtogroup DISPLAY
 * @{
 */

/** @defgroup STATUS_PAGE
 * Handle Status page
 * @{
 */

/* Private includes ----------------------------------------------------------*/
#include "statusPage.h"
#include "commonMethods.h"
#include <stdio.h>
#include "stm32f7xx_hal.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"
#define wait_delay HAL_Delay

/* Constants ------------------------------------------------------------------*/
/** @defgroup STATUS_PAGE_Private_Constants
 * Constants defined in statusPage.c
 * @{
 */
const int statusPage_lineYCoord[4] = {70, 105, 140, 175};  // Y position for each status text
const int statusPage_statusLabelXCoord = 10;               // X position when listing status labels (ex water leak, fire, etc.)
const int statusPage_statusXCoord = 305;                   // X position when listing status (ex Detected, none, etc.)
const int statusPage_buttonsXCoord[3] = {20, 190, 310};    // X positions for buttons on the status page
const int statusPage_buttonsYCoord = 220;                  // Y position for buttons on the status page
const int statusPage_buttonsHeight = 40;                   // Height for buttons
const int statusPage_buttonsWidth = 150;                   // Width for buttons
const int statusPage_lockButtonHeight = 40;             // Height for buttons
const int statusPage_lockButtonWidth = 105;             // Width for buttons
/**
 * @}
 */

/* Global Variables ------------------------------------------------------------------*/
/** @defgroup STATUS_PAGE_Private_Variables
 * Variables defined in statusPage.c
 * @{
 */
TS_StateTypeDef tsc_state_statusPage;
/**
 * @}
 */

/* Private define ------------------------------------------------------------*/
/** @defgroup STATUS_PAGE_Enum
 * Enums defined in statusPage.c
 * @{
 */

/**
 * @}
 */

/** @defgroup STATUS_PAGE_Private_Functions
 * Private Functions defined in statusPage.c
 * @{
 */

/**
 * @brief  Check what button has been clicked on by the user
 * @param  x X position of the touch
 * @param  y Y position of the touch
 * @retval Enum value of button touched
 */
enum StatusPageButtons statusPage_checkButtonCoords(int x, int y){
	if (isInRange(statusPage_buttonsXCoord[0], statusPage_buttonsXCoord[0] + statusPage_buttonsWidth, x) && isInRange(statusPage_buttonsYCoord, statusPage_buttonsYCoord + statusPage_buttonsHeight, y)){
		return MUTE;
	}
	else if (isInRange(statusPage_buttonsXCoord[1], statusPage_buttonsXCoord[1] + statusPage_lockButtonWidth, x) && isInRange(statusPage_buttonsYCoord, statusPage_buttonsYCoord + statusPage_lockButtonWidth, y)){
		return LOCK;
	}
	else if (isInRange(statusPage_buttonsXCoord[2], statusPage_buttonsXCoord[2] + statusPage_buttonsWidth, x) && isInRange(statusPage_buttonsYCoord, statusPage_buttonsYCoord + statusPage_buttonsHeight, y)){
		return DISCARD;
	}
	else if (isInRange(statusPage_statusXCoord, statusPage_statusXCoord + 160, x) && isInRange(statusPage_lineYCoord[2], statusPage_lineYCoord[2] + 24, y)){
		return DOOR_LOCK;
	}
	return NONE;
}
/**
 * @}
 */

/** @defgroup STATUS_PAGE_Public_Functions
 * Public Functions defined in statusPage.c
 * @{
 */
/**
 * @brief  Draw status page and all status labels
 * @param  muted Boolean indicating if device is currently muted or not
 * @retval None
 */
void displayStatusPage(bool muted){
	BSP_LCD_Clear(LCD_COLOR_WHITE);
	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_DisplayStringAt(120, 25, (uint8_t *)" Status Page ", LEFT_MODE);

	BSP_LCD_DisplayStringAt(statusPage_statusLabelXCoord, statusPage_lineYCoord[0], (uint8_t *)"Water Leak ", LEFT_MODE);
	BSP_LCD_DisplayStringAt(statusPage_statusLabelXCoord, statusPage_lineYCoord[1], (uint8_t *)"Fire Detected ", LEFT_MODE);
	BSP_LCD_DisplayStringAt(statusPage_statusLabelXCoord, statusPage_lineYCoord[2], (uint8_t *)"Front Door Lock ", LEFT_MODE);
	BSP_LCD_DisplayStringAt(statusPage_statusLabelXCoord, statusPage_lineYCoord[3], (uint8_t *)"Front Door Bell ", LEFT_MODE);

	BSP_LCD_SetTextColor(LCD_COLOR_LIGHTGRAY);
	BSP_LCD_DrawRect(statusPage_buttonsXCoord[0], statusPage_buttonsYCoord, statusPage_buttonsWidth, statusPage_buttonsHeight);
	BSP_LCD_DrawRect(statusPage_buttonsXCoord[1], statusPage_buttonsYCoord, statusPage_lockButtonWidth, statusPage_lockButtonHeight);
	BSP_LCD_DrawRect(statusPage_buttonsXCoord[2], statusPage_buttonsYCoord, statusPage_buttonsWidth, statusPage_buttonsHeight);

	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	if (muted){
		BSP_LCD_DisplayStringAt(statusPage_buttonsXCoord[0] + 10, statusPage_buttonsYCoord + 10, (uint8_t *)" Unmute", LEFT_MODE);
	} else {
		BSP_LCD_DisplayStringAt(statusPage_buttonsXCoord[0] + 15, statusPage_buttonsYCoord + 10, (uint8_t *)"  Mute ", LEFT_MODE);
	}
	BSP_LCD_DisplayStringAt(statusPage_buttonsXCoord[0] + 10, statusPage_buttonsYCoord + 10, (uint8_t *)" Unmute", LEFT_MODE);
	BSP_LCD_DisplayStringAt(statusPage_buttonsXCoord[1] + 6, statusPage_buttonsYCoord + 10, (uint8_t *)" Lock", LEFT_MODE);
	BSP_LCD_DisplayStringAt(statusPage_buttonsXCoord[2] + 8, statusPage_buttonsYCoord + 10, (uint8_t *)" Discard", LEFT_MODE);
}

/**
 * @brief  Update status page
 *
 * Update warnings for each label and the mute button (from mute to unmute or vice versa)
 *
 * @param  waterLeak Boolean indicating if the water leak warning needs to be displayed
 * @param  fire Boolean indicating if the fire warning needs to be displayed
 * @param  doorLocked Boolean indicating if the door is locked or not
 * @param  bellPressed Boolean indicating if the bell is pressed or not
 * @param  muted Boolean indicating if the user has muted the device or not
 * @retval None
 */
void updateStatusPage(bool waterLeak, bool fire, bool doorLocked, bool bellPressed, bool muted){
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);

	if (waterLeak){
		BSP_LCD_SetBackColor(LCD_COLOR_RED);
		BSP_LCD_DisplayStringAt(statusPage_statusXCoord, statusPage_lineYCoord[0], (uint8_t *)" Detected ", LEFT_MODE);
	} else {
		BSP_LCD_SetBackColor(LCD_COLOR_GREEN);
		BSP_LCD_DisplayStringAt(statusPage_statusXCoord, statusPage_lineYCoord[0], (uint8_t *)"   None   ",LEFT_MODE);
	}

	if (fire){
		BSP_LCD_SetBackColor(LCD_COLOR_RED);
		BSP_LCD_DisplayStringAt(statusPage_statusXCoord, statusPage_lineYCoord[1], (uint8_t *)" Detected ", LEFT_MODE);
	} else {
		BSP_LCD_SetBackColor(LCD_COLOR_GREEN);
		BSP_LCD_DisplayStringAt(statusPage_statusXCoord, statusPage_lineYCoord[1], (uint8_t *)"   None   ", LEFT_MODE);
	}

	if (doorLocked){
		BSP_LCD_SetBackColor(LCD_COLOR_GREEN);
		BSP_LCD_DisplayStringAt(statusPage_statusXCoord, statusPage_lineYCoord[2], (uint8_t *)"  Locked  ", LEFT_MODE);
	} else {
		BSP_LCD_SetBackColor(LCD_COLOR_RED);
		BSP_LCD_DisplayStringAt(statusPage_statusXCoord, statusPage_lineYCoord[2], (uint8_t *)" Unlocked ", LEFT_MODE);
	}

	if (bellPressed){
		BSP_LCD_SetBackColor(LCD_COLOR_RED);
		BSP_LCD_DisplayStringAt(statusPage_statusXCoord, statusPage_lineYCoord[3], (uint8_t *)"  Pressed ", LEFT_MODE);
	} else {
		BSP_LCD_SetBackColor(LCD_COLOR_GREEN);
		BSP_LCD_DisplayStringAt(statusPage_statusXCoord, statusPage_lineYCoord[3], (uint8_t *)"   None   ", LEFT_MODE);
	}

	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	if (muted){
		BSP_LCD_DisplayStringAt(statusPage_buttonsXCoord[0] + 10, statusPage_buttonsYCoord + 10, (uint8_t *)" Unmute", LEFT_MODE);
	} else {
		BSP_LCD_DisplayStringAt(statusPage_buttonsXCoord[0] + 15, statusPage_buttonsYCoord + 10, (uint8_t *)"  Mute ", LEFT_MODE);
	}
}

/**
 * @brief  Check if any buttons have been pressed on the status page
 * @param  None
 * @retval Enum value of the button pressed
 */
// Method to be called by the main method to check if the user has pressed any buttons on the status page
enum StatusPageButtons checkStatusPageTouch(void){
	enum StatusPageButtons retval;
	BSP_TS_GetState(&tsc_state_statusPage);

	// Check what button the user has tapped, if any
	if (tsc_state_statusPage.touchDetected){
		retval = statusPage_checkButtonCoords(tsc_state_statusPage.touchX[0], tsc_state_statusPage.touchY[0]);

		// Keep looping until user stops touching the screen to prevent accidental taps
		for(;;){
			BSP_TS_GetState(&tsc_state_statusPage);
			if(!tsc_state_statusPage.touchDetected) break;
		}
		return retval;
	}
	return NONE;
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
