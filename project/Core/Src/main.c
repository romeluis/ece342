/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//#include "ov7670.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum
{
  OFF = 0,
  ON
}PIN_VALUE;
typedef enum
{
  NONE = 0,
  NORTH,
  NORTHWEST,
  NORTHEAST,
  EAST,
  WEST,
  SURROUNDLEFT,
  SURROUNDRIGHT,
  SURROUND
}OBSTACLE_LOCATION;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SHORT_REVERSE 150
#define LONG_REVERSE 200
#define SHORT_TURN 300
#define QUARTER_TURN 400
#define HALF_TURN 450
#define TIME_THRESHOLD 500


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
struct SensorStatus {
	PIN_VALUE IR_Front;
	PIN_VALUE IR_Left;
	PIN_VALUE IR_Right;
	PIN_VALUE IR_Side_Left;
	PIN_VALUE IR_Side_Right;
};
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart3_tx;

PCD_HandleTypeDef hpcd_USB_OTG_FS;

/* USER CODE BEGIN PV */
uint8_t IR_SENSOR_LEFT_FLAG = 0;
uint8_t IR_SENSOR_FRONT_FLAG = 0;
uint8_t IR_SENSOR_RIGHT_FLAG = 0;

const uint8_t TURN_TIME = 100;
const uint8_t REVERSE_TIME = 100;
struct SensorStatus sensorStatus;
OBSTACLE_LOCATION currentObstacle = NONE;
OBSTACLE_LOCATION previousObstacle = NONE;
uint32_t currentObstacleTime = 0;
uint32_t previousObstacleTime = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_USB_OTG_FS_PCD_Init(void);
/* USER CODE BEGIN PFP */
void extraTruncatedVideoMode();
uint8_t findNumberOfRepeatedPixels(uint8_t startRow, uint8_t startCol);
void transferRawBufferToIntermediate();
int rle(int bufferIndex);
void vehicleForward();
void vehicleForwardUntilUnClear();
void vehicleStop();
void vehicleReverse();
void vehicleReverseUntilClear();
void vehicleLeft();
void vehicleLeftUntilClear();
void vehicleRight();
void vehicleRightUntilClear();
//MOTOR HELPER
void motor1CW();
void motor1CCW();
void motor2CW();
void motor2CCW();
void motor3CW();
void motor3CCW();
void motor4CW();
void motor4CCW();
//Motor Pin Output
void writeMotor1Pin1(PIN_VALUE state);
void writeMotor1Pin2(PIN_VALUE state);
void writeMotor2Pin1(PIN_VALUE state);
void writeMotor2Pin2(PIN_VALUE state);
void writeMotor3Pin1(PIN_VALUE state);
void writeMotor3Pin2(PIN_VALUE state);
void writeMotor4Pin1(PIN_VALUE state);
void writeMotor4Pin2(PIN_VALUE state);
//Sensor Helper
void refreshSensorStatus();
int isLeftBlocked();
int isRightBlocked();
int isFrontBlocked();
int isSideLeftBlocked();
int isSideRightBlocked();
int canContinueForward();
//OBSTACLE HELPER
void findObstacle();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  /* USER CODE BEGIN 2 */
  vehicleStop();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while(!HAL_GPIO_ReadPin(USER_Btn_GPIO_Port, USER_Btn_Pin)) {
	  HAL_Delay(10);
  }

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  //Continue forward until obstacle detected
	  vehicleForwardUntilUnClear();

	  //***OBSTACLE DETECTED***//
	  currentObstacleTime = HAL_GetTick();
	  //Determine Location of obstacle
	  findObstacle();
	  previousObstacle = currentObstacle;

	  //Feedback loop escape
	  if (currentObstacleTime - previousObstacleTime <= TIME_THRESHOLD) {
		  vehicleReverse();
		  HAL_Delay(LONG_REVERSE);
		  vehicleStop();
		  if (HAL_GetTick() % 2 == 0) {
			  vehicleLeft();
		  } else {
			  vehicleRight();
		  }
		  HAL_Delay(HALF_TURN);
	  }
	  previousObstacleTime = currentObstacleTime;
	  //Navigate Obstacle
	  if (currentObstacle == NONE) { //Nothing detected
		  //do nothing
		  //Error_Handler();
	  } else if (currentObstacle == NORTH) {
		  vehicleReverse();
		  HAL_Delay(SHORT_REVERSE);
		  vehicleStop();
		  if (HAL_GetTick() % 2 == 0) {
			  vehicleLeftUntilClear();
		  } else {
			  vehicleRightUntilClear();
		  }
	  } else if (currentObstacle == NORTHWEST) {
		  vehicleRightUntilClear();
	  } else if (currentObstacle == NORTHEAST) {
		  vehicleLeftUntilClear();
	  } else if (currentObstacle == WEST) {
		  vehicleReverse();
		  HAL_Delay(SHORT_REVERSE);
		  vehicleStop();

		  vehicleRight();
		  HAL_Delay(QUARTER_TURN);
	  } else if (currentObstacle == EAST) {
		  vehicleReverse();
		  HAL_Delay(SHORT_REVERSE);
		  vehicleStop();

		  vehicleLeft();
		  HAL_Delay(QUARTER_TURN);
	  } else if (currentObstacle == SURROUNDLEFT) {
		  vehicleReverseUntilClear();

		  vehicleRight();
		  HAL_Delay(QUARTER_TURN);
	  } else if (currentObstacle == SURROUNDRIGHT) {
		  vehicleReverseUntilClear();

		  vehicleLeft();
		  HAL_Delay(QUARTER_TURN);
	  } else if (currentObstacle == SURROUND) {
		  vehicleReverseUntilClear();

		  if (HAL_GetTick() % 2 == 0) {
			  vehicleLeft();
		  } else {
			  vehicleRight();
		  }
		  HAL_Delay(QUARTER_TURN);

	  }

	  //Small delay before continuing
	  vehicleStop();
	  HAL_Delay(500);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
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
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief USB_OTG_FS Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_OTG_FS_PCD_Init(void)
{

  /* USER CODE BEGIN USB_OTG_FS_Init 0 */

  /* USER CODE END USB_OTG_FS_Init 0 */

  /* USER CODE BEGIN USB_OTG_FS_Init 1 */

  /* USER CODE END USB_OTG_FS_Init 1 */
  hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
  hpcd_USB_OTG_FS.Init.dev_endpoints = 6;
  hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_OTG_FS.Init.Sof_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.vbus_sensing_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_OTG_FS_Init 2 */

  /* USER CODE END USB_OTG_FS_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, MOTOR_1_OUT_1_Pin|MOTOR_1_OUT_2_Pin|MOTOR_2_OUT_1_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(MOTOR_2_OUT_2_GPIO_Port, MOTOR_2_OUT_2_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, MOTOR_3_OUT_1_Pin|MOTOR_3_OUT_2_Pin|MOTOR_4_OUT_1_Pin|MOTOR_4_OUT_2_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USER_Btn_Pin */
  GPIO_InitStruct.Pin = USER_Btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : MOTOR_1_OUT_1_Pin MOTOR_2_OUT_1_Pin */
  GPIO_InitStruct.Pin = MOTOR_1_OUT_1_Pin|MOTOR_2_OUT_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : MOTOR_1_OUT_2_Pin */
  GPIO_InitStruct.Pin = MOTOR_1_OUT_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(MOTOR_1_OUT_2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD1_Pin|LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : MOTOR_2_OUT_2_Pin */
  GPIO_InitStruct.Pin = MOTOR_2_OUT_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(MOTOR_2_OUT_2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : IR_SENSOR_IN_SIDE_RIGHT_Pin IR_SENSOR_IN_LEFT_Pin IR_SENSOR_IN_RIGHT_Pin USB_OverCurrent_Pin */
  GPIO_InitStruct.Pin = IR_SENSOR_IN_SIDE_RIGHT_Pin|IR_SENSOR_IN_LEFT_Pin|IR_SENSOR_IN_RIGHT_Pin|USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : MOTOR_3_OUT_1_Pin MOTOR_3_OUT_2_Pin MOTOR_4_OUT_1_Pin MOTOR_4_OUT_2_Pin */
  GPIO_InitStruct.Pin = MOTOR_3_OUT_1_Pin|MOTOR_3_OUT_2_Pin|MOTOR_4_OUT_1_Pin|MOTOR_4_OUT_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : IR_SENSOR_IN_FRONT_Pin IR_SENSOR_IN_SIDE_LEFT_Pin */
  GPIO_InitStruct.Pin = IR_SENSOR_IN_FRONT_Pin|IR_SENSOR_IN_SIDE_LEFT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
//VEHICLE HELPER
void vehicleForward() {
	motor1CCW();
	motor3CCW();
	motor2CW();
	motor4CW();
}
void vehicleForwardUntilUnClear() {
	refreshSensorStatus();
	while(canContinueForward()) {
		vehicleForward();
		HAL_Delay(0);
		refreshSensorStatus();
	}
	vehicleStop();

}
void vehicleStop() {
	writeMotor1Pin1(OFF);
	writeMotor1Pin2(OFF);
	writeMotor2Pin1(OFF);
	writeMotor2Pin2(OFF);
	writeMotor3Pin1(OFF);
	writeMotor3Pin2(OFF);
	writeMotor4Pin1(OFF);
	writeMotor4Pin2(OFF);
}
void vehicleReverse() {
	motor1CW();
	motor3CW();
	motor2CCW();
	motor4CCW();
}
void vehicleLeft() {
	motor1CW();
	motor3CW();
	motor2CW();
	motor4CW();
}
void vehicleLeftUntilClear() {
	vehicleLeft();
	while(isRightBlocked()) {
		vehicleLeft();
		HAL_Delay(10);
		refreshSensorStatus();
	}
	HAL_Delay(SHORT_TURN); //Some additional radius
	vehicleStop();

}
void vehicleRight() {
	motor2CCW();
	motor4CCW();
	motor1CCW();
	motor3CCW();
}
void vehicleRightUntilClear() {
	vehicleRight();
	while(isLeftBlocked()) {
		vehicleRight();
		HAL_Delay(10);
		refreshSensorStatus();
	}
	HAL_Delay(SHORT_TURN); //Some additional radius
	vehicleStop();

}
void vehicleReverseUntilClear() {
	while(!canContinueForward() && isSideRightBlocked() && isSideLeftBlocked()) {
		vehicleReverse();
		HAL_Delay(10);
		refreshSensorStatus();
	}
	vehicleStop();

}
//MOTOR HELPER
void motor1CW() {
	writeMotor1Pin1(ON);
	writeMotor1Pin2(OFF);
}
void motor1CCW() {
	writeMotor1Pin1(OFF);
	writeMotor1Pin2(ON);
}
void motor2CW() {
	writeMotor2Pin1(ON);
	writeMotor2Pin2(OFF);
}
void motor2CCW() {
	writeMotor2Pin1(OFF);
	writeMotor2Pin2(ON);
}
void motor3CW() {
	writeMotor3Pin1(ON);
	writeMotor3Pin2(OFF);
}
void motor3CCW() {
	writeMotor3Pin1(OFF);
	writeMotor3Pin2(ON);
}
void motor4CW() {
	writeMotor4Pin1(ON);
	writeMotor4Pin2(OFF);
}
void motor4CCW() {
	writeMotor4Pin1(OFF);
	writeMotor4Pin2(ON);
}
//Motor Pin Output
void writeMotor1Pin1(PIN_VALUE state) {
	HAL_GPIO_WritePin(GPIOF, MOTOR_1_OUT_1_Pin, state);
}
void writeMotor1Pin2(PIN_VALUE state) {
	HAL_GPIO_WritePin(GPIOF, MOTOR_1_OUT_2_Pin, state);
}
void writeMotor2Pin1(PIN_VALUE state) {
	HAL_GPIO_WritePin(GPIOF, MOTOR_2_OUT_1_Pin, state);
}
void writeMotor2Pin2(PIN_VALUE state) {
	HAL_GPIO_WritePin(MOTOR_2_OUT_2_GPIO_Port, MOTOR_2_OUT_2_Pin, state);
}
void writeMotor3Pin1(PIN_VALUE state) {
	HAL_GPIO_WritePin(GPIOE, MOTOR_3_OUT_1_Pin, state);
}
void writeMotor3Pin2(PIN_VALUE state) {
		HAL_GPIO_WritePin(GPIOE, MOTOR_3_OUT_2_Pin, state);
}
void writeMotor4Pin1(PIN_VALUE state) {
	HAL_GPIO_WritePin(GPIOE, MOTOR_4_OUT_1_Pin, state);
}
void writeMotor4Pin2(PIN_VALUE state) {
	HAL_GPIO_WritePin(GPIOE, MOTOR_4_OUT_2_Pin, state);
}
//Sensor Helper
void refreshSensorStatus() {
	sensorStatus.IR_Left = !HAL_GPIO_ReadPin(IR_SENSOR_IN_LEFT_GPIO_Port, IR_SENSOR_IN_LEFT_Pin);
	sensorStatus.IR_Right = !HAL_GPIO_ReadPin(IR_SENSOR_IN_RIGHT_GPIO_Port, IR_SENSOR_IN_RIGHT_Pin);
	sensorStatus.IR_Front = !HAL_GPIO_ReadPin(IR_SENSOR_IN_FRONT_GPIO_Port, IR_SENSOR_IN_FRONT_Pin);
	sensorStatus.IR_Side_Left = !HAL_GPIO_ReadPin(IR_SENSOR_IN_SIDE_LEFT_GPIO_Port, IR_SENSOR_IN_SIDE_LEFT_Pin);
	sensorStatus.IR_Side_Right = !HAL_GPIO_ReadPin(IR_SENSOR_IN_SIDE_RIGHT_GPIO_Port, IR_SENSOR_IN_SIDE_RIGHT_Pin);
}
int isLeftBlocked() {
	return sensorStatus.IR_Left;
}
int isRightBlocked() {
	return sensorStatus.IR_Right;
}
int isFrontBlocked() {
	return sensorStatus.IR_Front;
}
int isSideLeftBlocked() {
	return sensorStatus.IR_Side_Left;
}
int isSideRightBlocked() {
	return sensorStatus.IR_Side_Right;
}
int canContinueForward() {
	return !(isRightBlocked() || isLeftBlocked() || isFrontBlocked());
}
//OBSTACLE HELPER
void findObstacle() {
	refreshSensorStatus();

	//Check Front (north)
	if ((isFrontBlocked() && !isRightBlocked() && !isLeftBlocked()) || (isFrontBlocked() && isRightBlocked() && isLeftBlocked()) || (!isFrontBlocked() && isRightBlocked() && isLeftBlocked())) {
		//Check sides
		if (!isSideRightBlocked() && !isSideLeftBlocked()) { //Obstacle is directly in front
			currentObstacle = NORTH;
			return;
		}
		if (isSideRightBlocked() && isSideLeftBlocked()) { //Obstacle completely surrounded
			currentObstacle = SURROUND;
			return;
		}
		if (!isSideRightBlocked() && isSideLeftBlocked()) { //Obstacle in front and left
			if (previousObstacle == SURROUNDRIGHT) {
				currentObstacle = SURROUND;
			} else {
				currentObstacle = SURROUNDLEFT;
			}
			return;
		}
		if (isSideRightBlocked() && !isSideLeftBlocked()) { //Obstacle in front and right
			if (previousObstacle == SURROUNDLEFT) {
				currentObstacle = SURROUND;
			} else {
				currentObstacle = SURROUNDRIGHT;
			}
			return;
		}
	}
	//Check right (east)
	if ((isFrontBlocked() && isRightBlocked() && !isLeftBlocked()) || (!isFrontBlocked() && isRightBlocked() && !isLeftBlocked())) {
		//Check sides
		if (!isSideRightBlocked() && !isSideLeftBlocked()) { //Obstacle right in front slightly to the right
			currentObstacle = NORTHEAST;
			return;
		}
		if (isSideRightBlocked() && isSideLeftBlocked()) { //Obstacle completely surrounds
			currentObstacle = SURROUND;
			return;
		}
		if (!isSideRightBlocked() && isSideLeftBlocked()) { //Obstacle weirdly surrounds
			currentObstacle = SURROUND;
			return;
		}
		if (isSideRightBlocked() && !isSideLeftBlocked()) { //Obstacle in front and right
			currentObstacle = EAST;
			return;
		}
	}
	//Check left (west)
	if ((isFrontBlocked() && !isRightBlocked() && isLeftBlocked()) || (!isFrontBlocked() && !isRightBlocked() && isLeftBlocked())) {
		//Check sides
		if (!isSideRightBlocked() && !isSideLeftBlocked()) { //Obstacle right in front and slightly to the left
			currentObstacle = NORTHWEST;
			return;
		}
		if (isSideRightBlocked() && isSideLeftBlocked()) { //Obstacle completely surrounds
			currentObstacle = SURROUND;
			return;
		}
		if (!isSideRightBlocked() && isSideLeftBlocked()) { //Obstacle in front and left
			currentObstacle = WEST;
			return;
		}
		if (isSideRightBlocked() && !isSideLeftBlocked()) { //Obstacle weirdly surrounds
			currentObstacle = SURROUND;
			return;
		}
	}
	currentObstacle = NONE;
}
//CAMERA HELPER
/*
void extraTruncatedVideoMode() {
	int bufferIndex= 0;
	for (int index = 0; index < sizeof(PREAMBLE); index++) {
		video_buff[bufferIndex++] = PREAMBLE[index];
	}

	//Loop forever
	while(1) {
		//Reset
		dma_flag = 0;
		ov7670_capture(raw_buff);

		//Wait for capture to finish
		while (!dma_flag) {
			HAL_Delay(0);
		}

		//Extract grey scale
		transferRawBufferToIntermediate();

		//RLE and place between pre-amble and suffix
		bufferIndex = rle(sizeof(PREAMBLE));

		//Append suffix
		for (int index = 0; index < sizeof(SUFFIX); index++) {
			video_buff[bufferIndex] = SUFFIX[index];
			bufferIndex++;
		}

		//Export Data
		if (HAL_UART_GetState(&huart3) == HAL_UART_STATE_READY) {
			HAL_UART_Transmit_DMA(&huart3, (uint8_t *)video_buff, bufferIndex);
		}

	}

}
//Extracts grey-scale data from the raw buffer to the intermediate buffer and formats it in a 2D array
void transferRawBufferToIntermediate() {
	int rowIndex = 0;
	int colIndex = 0;
	for (int rawIndex = 1; rawIndex < 2*IMG_ROWS*IMG_COLS; rawIndex++) {
		if (rawIndex % 2 != 0) {
			intermediate_buff[rowIndex][colIndex++] = raw_buff[rawIndex] & 0xF0;
			if (colIndex == IMG_COLS) {
				rowIndex++;
				colIndex = 0;
			}
		}
	}
}
//Returns the number of pixels that are consecutive, beginning search at [startRow, startCol]
uint8_t findNumberOfRepeatedPixels(uint8_t startRow, uint8_t startCol) {
	uint8_t count = 0;
	uint8_t prevColour = intermediate_buff[startRow][startCol];
	for (int columnIndex = startCol; columnIndex < IMG_COLS && count < 15; columnIndex++) {
		if (intermediate_buff[startRow][columnIndex] == prevColour) {
			count++;
		} else {
			break;
		}
	}
	return count;
}
//Compresses contents of intermediate buff using RLE and places data in the output buffer beginning at bufferIndex
int rle(int bufferIndex) {
	for (int rowIndex = 0; rowIndex < IMG_ROWS; rowIndex++) {
		for (int colIndex = 0; colIndex < IMG_COLS;) {
			uint8_t repetitions = findNumberOfRepeatedPixels(rowIndex, colIndex);

			video_buff[bufferIndex++] = intermediate_buff[rowIndex][colIndex] | (repetitions & 0xF);

			colIndex += repetitions;
		}
	}
	return bufferIndex;
}
*/
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
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
