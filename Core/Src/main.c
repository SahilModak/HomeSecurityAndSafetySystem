/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Sahil Modak & Ali Nanji
 * @brief          : Main program body
 *
 * Contains application entry point and main infinite loop
 ******************************************************************************
 */

/** @addtogroup CORE
 * All modules implemented by us excluding all libraries
 * @{
 */

/** @defgroup MAIN
 * @brief main file
 *
 * Contains application entry point and main loop
 * @{
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "pinpad.h"
#include "statusPage.h"
#include "commonMethods.h"
#include "membrane.h"
#include "adcSensors.h"
#include "servoMotor.h"
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/** @defgroup MAIN_Enum
 * Enum defined in main.c
* @{
*/
/*!
 * @brief Enum defined to keep track of current page on UI
 * */
enum CurrPage {
	PINPAD, /**< Indicates that the current UI page is the PINPAD page */
	STATUS  /**< Indicates that the UI is currently displaying the STATUS page */
};
/**
* @}
*/

/* USER CODE END PD */

/* USER CODE BEGIN PV */
/* Constants ------------------------------------------------------------------*/
/** @defgroup MAIN_Private_Constants
 * Constants defined in main.c
* @{
*/
const uint8_t BT_TX_MUTE = 0;
const uint8_t BT_TX_WATER_DETECTED = 25;
const uint8_t BT_TX_WATER_NOT_DETECTED = 26;
const uint8_t BT_TX_FIRE_DETECTED = 50;
const uint8_t BT_TX_FIRE_NOT_DETECTED = 51;
const uint8_t BT_TX_LOCKED = 75;
const uint8_t BT_TX_UNLOCKED = 76;
const uint8_t BT_TX_BELL_PRESSED = 100;
const uint8_t BT_TX_BELL_NOT_PRESSED = 101;
const uint8_t BT_TX_DISCARD = 125;
const uint8_t BT_RX_MUTE = 50;
const uint8_t BT_RX_DISCARD = 100;
const int SCREEN_PIN_LEN = 4;
const int MEMBRANE_PIN_LEN = 4;
const int DISCARDED_WARNING_WAIT_PERIOD = 5000;
/**
* @}
*/

/* Global Variables ------------------------------------------------------------------*/
/** @defgroup MAIN_Private_Variables
 * Private Variables defined in main.c
* @{
*/

/* Buffer used to receive Bluetooth messages*/
uint8_t RX_BUFFER;

CRC_HandleTypeDef hcrc;
DMA2D_HandleTypeDef hdma2d;
UART_HandleTypeDef huart7;
ADC_HandleTypeDef hadc;
GPIO_InitTypeDef gpio_buzzer;
GPIO_InitTypeDef gpio_touchSensor;
TIM_HandleTypeDef htim3;

TS_StateTypeDef  ts;
char xTouchStr[10];

bool waterDetected = false;
bool fireDetected = false;
bool doorLocked = true;
bool bellPressed = false;
bool muted = false;

bool btPreviousWaterTr = false;
bool btPreviousFireTr = false;
bool btPreviousLockTr = true;
bool btPreviousBellTr = false;

int lastBtTx = 0;
int lastBtTx_spam = 0;

bool showWaterWarning = false;
bool showFireWarning = false;
bool showDoorLockWarning = false;
bool showDoorBellWarning = false;

int doorLock_discardedWarningWait = 0;
int doorBell_discardedWarningWait = 0;
int water_discardedWarningWait = 0;
int fire_discardedWarningWait = 0;
/**
* @}
*/
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/** @defgroup MAIN_Private_FunctionPrototypes
 * Function Prototypes for functions in main.c
* @{
*/
void SystemClock_Config(void);
static void MX_CRC_Init(void);
static void MX_DMA2D_Init(void);
static void MX_UART7_Init(void);
static void initializeTouchSensor(GPIO_InitTypeDef *gpio);
static void initializeBuzzer(GPIO_InitTypeDef *gpio);
static void updateAndroidApp(void);
static void discardCurrentWarnings(void);
static int msPassedSince(int time);
static void btTransmit(uint8_t *value);
/**
* @}
*/

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/** @defgroup MAIN_Private_Functions
 * Private Functions defined in main.c
* @{
*/

/**
 * @brief  The application entry point.
 * @param  None
 * @retval int
 */
int main(void)
{
	/* USER CODE BEGIN 1 */
	enum StatusPageButtons buttonPressed = NONE;

	int currentDispDigitCount = 0;
	int currentDispDigits[SCREEN_PIN_LEN];

	int currentMemDigitCount = 0;
	int currentMemPin = 0;

	enum CurrPage currPage = PINPAD;

	/* USER CODE BEGIN 1 */

	/* Enable the CPU Cache */
	/* Enable I-Cache */
	SCB_EnableICache();
	/* Enable D-Cache */
	SCB_EnableDCache();

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* STM32F7xx HAL library initialization:
	  - Configure the Flash ART accelerator on ITCM interface
	  - Configure the Systick to generate an interrupt each 1 msec
	  - Set NVIC Group Priority to 4
	  - Global MSP (MCU Support Package) initialization
	  - Reset of all peripherals
	  */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_CRC_Init();
	MX_DMA2D_Init();
	MX_UART7_Init();
	enableClocks();
	initializeMembranePins();
	initializeAdcSensors(&hadc);
	initializeBuzzer(&gpio_buzzer);
	initializeTouchSensor(&gpio_touchSensor);
	initServoMotor(&htim3);

	/* USER CODE BEGIN 2 */
	/* Initializes the SDRAM device */
	BSP_SDRAM_Init();

	/*Initializes the lcd to show lock screen*/
	initializeDisplay();
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	HAL_UART_Receive_IT(&huart7, &RX_BUFFER, 1);
	uint8_t x = 79;
	while (1)
	{
		/* Checking if the membrane keypad has been pressed. If it is, then unlock door*/
		if(checkMemPin(&currentMemDigitCount, &currentMemPin, MEMBRANE_PIN_LEN)){
			doorLocked = false;
			unlockDoor(&htim3);
			wait(1);
		}

		/* Check if any of the sensors have been activated */
		waterDetected = checkWaterDetected(&hadc);
		fireDetected = checkFireDetected(&hadc);
		bellPressed = (HAL_GPIO_ReadPin(GPIOC, gpio_touchSensor.Pin) == GPIO_PIN_SET);

		/* Ignore warnings if the warning wait is not 0 */
		showWaterWarning = waterDetected && water_discardedWarningWait == 0;
		showFireWarning = fireDetected && fire_discardedWarningWait == 0;
		showDoorLockWarning = !doorLocked && doorLock_discardedWarningWait == 0;
		showDoorBellWarning = bellPressed && doorBell_discardedWarningWait == 0;

		// Perform actions based on what the current page is
		if (currPage == PINPAD){
			// Check if the pin has been input on the LCD screen
			bool correctPassword = checkPin(&currentDispDigitCount, currentDispDigits);
			if (correctPassword){
				currPage = STATUS;
				displayStatusPage(muted);
			}
		}
		else if (currPage == STATUS){
			// Update current warnings based on booleans calculated above
			updateStatusPage(showWaterWarning, showFireWarning, doorLocked, showDoorBellWarning, muted);

			// Check if any buttons on the status page have been pressed and do according action
			buttonPressed = checkStatusPageTouch();
			if (buttonPressed == MUTE){
				// Mute the buzzer if the user taps on the mute button
				muted = !muted;
				btTransmit(&BT_TX_MUTE);
			}
			else if (buttonPressed == LOCK){
				// Go back to the lock screen if the user taps on the lock button
				currPage = PINPAD;
				displayLockScreen();
			}
			else if (buttonPressed == DISCARD){
				discardCurrentWarnings();
				btTransmit(&BT_TX_DISCARD);
			}
			else if (buttonPressed == DOOR_LOCK){
				// If the user taps on the front door lock button, lock or unlock door based on current status
				if (doorLocked){
					unlockDoor(&htim3);
				} else {
					lockDoor(&htim3);
				}
				doorLocked = !doorLocked;
			}
		}

		// Buzzer only buzzes if the user hasn't currently unmuted the system
		if (!muted && (showWaterWarning || showFireWarning || showDoorLockWarning || showDoorBellWarning)){
			HAL_GPIO_WritePin(GPIOC, gpio_buzzer.Pin, GPIO_PIN_SET);
		} else {
			HAL_GPIO_WritePin(GPIOC, gpio_buzzer.Pin, GPIO_PIN_RESET);
		}

		// Send data to Android APP if its been half a second since the last data transmit
		if (msPassedSince(lastBtTx) >= 500){
			updateAndroidApp();
			lastBtTx = HAL_GetTick();
		}

		/* Transmits a random value every 5 secs to allow android app to connect */
		if (msPassedSince(lastBtTx_spam) >= 5000){
			btTransmit(&x);
			lastBtTx_spam = HAL_GetTick();
		}

		// If any of the warning are currently discarded, decrement the counter by 1 per loop until they reach 0
		if (doorLock_discardedWarningWait > 0) doorLock_discardedWarningWait--;
		if (doorBell_discardedWarningWait > 0) doorBell_discardedWarningWait--;
		if (water_discardedWarningWait > 0) water_discardedWarningWait--;
		if (fire_discardedWarningWait > 0) fire_discardedWarningWait--;
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
/**
 * @brief  System Clock Configuration
 *         The system Clock is configured as follow :
 *            System Clock source            = PLL (HSE)
 *            SYSCLK(Hz)                     = 200000000
 *            HCLK(Hz)                       = 200000000
 *            AHB Prescaler                  = 1
 *            APB1 Prescaler                 = 4
 *            APB2 Prescaler                 = 2
 *            HSE Frequency(Hz)              = 25000000
 *            PLL_M                          = 12
 *            PLL_N                          = 192
 *            PLL_P                          = 2
 *            PLL_Q                          = 9
 *            PLLSAI_P                       = 8
 *            VDD(V)                         = 3.3
 *            Main regulator output voltage  = Scale1 mode
 *            Flash Latency(WS)              = 6
 * @param  None
 * @retval None
 */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 12;
	RCC_OscInitStruct.PLL.PLLN = 192;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 9;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Activate the Over-Drive mode
	 */
	if (HAL_PWREx_EnableOverDrive() != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
			|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
	{
		Error_Handler();
	}
}

/**
 * @brief CRC Initialization Function
 *         The CRC is configured as follow :
 *            Polynomial Use                 = Default
 *            Init value use                 = Default
 *            Input Data Inversion Mode      = None
 *            Output Data Inversion Mode     = Disable
 *            Input Data Format                 = 4
 * @param None
 * @retval None
 */
static void MX_CRC_Init(void)
{
	hcrc.Instance = CRC;
	hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
	hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
	hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
	hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
	hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
	if (HAL_CRC_Init(&hcrc) != HAL_OK)
	{
		Error_Handler();
	}

}

/**
 * @brief DMA2D Initialization Function
 * @param None
 * @retval None
 */
static void MX_DMA2D_Init(void)
{
	hdma2d.Instance = DMA2D;
	hdma2d.Init.Mode = DMA2D_M2M;
	hdma2d.Init.ColorMode = DMA2D_OUTPUT_ARGB8888;
	hdma2d.Init.OutputOffset = 0;
	hdma2d.LayerCfg[1].InputOffset = 0;
	hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
	hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	hdma2d.LayerCfg[1].InputAlpha = 0;
	if (HAL_DMA2D_Init(&hdma2d) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK)
	{
		Error_Handler();
	}

}

/**
 * @brief UART7 Initialization Function
 * @param None
 * @retval None
 */
static void MX_UART7_Init(void)
{
	huart7.Instance = UART7;
	huart7.Init.BaudRate = 9600;
	huart7.Init.WordLength = UART_WORDLENGTH_8B;
	huart7.Init.StopBits = UART_STOPBITS_1;
	huart7.Init.Parity = UART_PARITY_NONE;
	huart7.Init.Mode = UART_MODE_TX_RX;
	huart7.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart7.Init.OverSampling = UART_OVERSAMPLING_16;
	huart7.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart7.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart7) != HAL_OK)
	{
		Error_Handler();
	}
}

/**
 * @brief Buzzer GPIO Initialization Function
 * 	      The buzzer uses pin D1 on the board. It is configured as follows:
 * 	           Mode = OUTPUT PUSH PULL
 * 	           Pull = PULL DOWN
 * 	           Speed = SPEED HIGH
 * 	           Pin = PIN 6
 *
 * @param gpio: Pointer to the buzzer GPIO Init structure
 * @retval None
 */

void initializeBuzzer(GPIO_InitTypeDef *gpio){
	gpio->Mode = GPIO_MODE_OUTPUT_PP;
	gpio->Pull = GPIO_PULLDOWN;
	gpio->Speed = GPIO_SPEED_HIGH;
	gpio->Pin = GPIO_PIN_6;

	HAL_GPIO_Init(GPIOC, gpio);
}

/**
 * @brief Touch Sensor GPIO Initialization Function
 * 	      The touch sensor uses pin D0 on the board. It is configured as follows:
 * 	           Mode = Input
 * 	           Pull = PULL DOWN
 * 	           Speed = SPEED HIGH
 * 	           Pin = PIN 7
 *
 * @param gpio: Pointer to the touch sensor GPIO Init structure
 * @retval None
 */
void initializeTouchSensor(GPIO_InitTypeDef *gpio){
	gpio->Mode = GPIO_MODE_INPUT;
	gpio->Pull = GPIO_PULLDOWN;
	gpio->Speed = GPIO_SPEED_HIGH;
	gpio->Pin = GPIO_PIN_7;

	HAL_GPIO_Init(GPIOC, gpio);
}

/*
 * @brief Transmit current warnings to android app
 * Compare each warning type's current status with its last transmitted
 * value, and only if they differ will it transmit the corresponding warning
 *
 * @param None
 * @retval None
 * */

void updateAndroidApp(void){
	if (btPreviousWaterTr != showWaterWarning){
		showWaterWarning ? btTransmit(&BT_TX_WATER_DETECTED) : btTransmit(&BT_TX_WATER_NOT_DETECTED);
		btPreviousWaterTr = showWaterWarning;
	}

	if (btPreviousFireTr != showFireWarning){
		showFireWarning ? btTransmit(&BT_TX_FIRE_DETECTED) : btTransmit(&BT_TX_FIRE_NOT_DETECTED);
		btPreviousFireTr = showFireWarning;
	}

	if (btPreviousLockTr != doorLocked){
		doorLocked ? btTransmit(&BT_TX_LOCKED) : btTransmit(&BT_TX_UNLOCKED);
		btPreviousLockTr = doorLocked;
	}

	if (btPreviousBellTr != showDoorBellWarning){
		showDoorBellWarning ? btTransmit(&BT_TX_BELL_PRESSED) : btTransmit(&BT_TX_BELL_NOT_PRESSED);
		btPreviousBellTr = showDoorBellWarning;
	}
}

/*
 * @brief Discard Current Warnings
 * For each active warning, add a timeout that will prevent showing
 * warning for those warning types
 *
 * @param None
 * @retval None
 * */
void discardCurrentWarnings(void){
	// If the user taps on the discard button, set a timer for all current warnings
	if (showWaterWarning)    water_discardedWarningWait    = DISCARDED_WARNING_WAIT_PERIOD;
	if (showFireWarning)     fire_discardedWarningWait     = DISCARDED_WARNING_WAIT_PERIOD;
	if (showDoorLockWarning) doorLock_discardedWarningWait = DISCARDED_WARNING_WAIT_PERIOD;
	if (showDoorBellWarning) doorBell_discardedWarningWait = DISCARDED_WARNING_WAIT_PERIOD;
}

/*
 * @brief Calculate milliseconds elapsed since the provided time
 *
 * @param time The time to calculate the duration from
 * @retval Milliseconds elapsed since the provided time
 * */
int msPassedSince(int time){
	int currentTime = HAL_GetTick();
	return currentTime - time;
}

/*
 * @brief Wrapper function to transmit data over bluetooth
 *
 * @param value Point to data to be transmitted
 * @retval None
 * */
void btTransmit(uint8_t *value){
	uint16_t size = 1;
	uint32_t timeout = 1000;
	HAL_UART_Transmit(&huart7, value, size, timeout);
}


/*
 * @brief Callback function for data received over bluetooth
 *
 * @param huart Pointer to UART Handle Structure
 * @retval None
 * */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == UART7)
	{
		if(RX_BUFFER == BT_RX_MUTE)
		{
			muted = !muted;
		}
		else if (RX_BUFFER == BT_RX_DISCARD)
		{
			discardCurrentWarnings();
		}

		HAL_UART_Receive_IT(&huart7, &RX_BUFFER, 1);
	}
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @param None
 * @retval None
 */
void Error_Handler(void)
{
	__disable_irq();
	while (1)
	{
	}
}

#ifdef  USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
* @}
*/

/**
* @}
*/

/**
* @}
*/

