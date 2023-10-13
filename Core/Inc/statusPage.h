/**
 **********************************************************************************************************
 * @file           : statusPage.h
 * @author         : Ali Nanji
 * @brief          : Header file for statusPage.c.
 *
 * This file contains the function prototypes for all public functions in statusPage.c, and also defines
 * the StatusPageButtons Enum.
 **********************************************************************************************************
 */

#include <stdbool.h>

/** @addtogroup STATUS_PAGE_Enum
 * @{
 */

/*!
 * @brief Enum defined to communicate what button has been pressed on the Status page
 * */
enum StatusPageButtons {
	LOCK,      /**< Lock display screen */
	MUTE,      /**< Mute Warnings (Display warnings but mute sound) */
	DISCARD,   /**< Discard Warnings (Hide all warnings. Door Lock warning can not be discarded) */
	DOOR_LOCK, /**< Lock Front Door */
	NONE       /**< No Button Pressed */
};
/**
* @}
*/

// Display basic status page without any warnings
void displayStatusPage(bool muted);

// Update status page with warnings
void updateStatusPage(bool waterLeak, bool fire, bool doorLocked, bool bellPressed, bool muted);

// Check if any button on the status page has been clicked on and return enum defined above accordingly
enum StatusPageButtons checkStatusPageTouch(void);
