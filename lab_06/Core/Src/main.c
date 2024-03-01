/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include <string.h>
#include <stdio.h>
#include "config.h"
#include "ov7670.h"


/* USER CODE BEGIN PV */
#define PREAMBLE "\r\n!START!\r\n"
#define DELTA_PREAMBLE "\r\n!DELTA!\r\n"
#define SUFFIX "!END!\r\n"

uint8_t raw_buff[2 * IMG_ROWS * IMG_COLS];

uint8_t intermediate_buff[IMG_ROWS][IMG_COLS];

uint8_t previous_buff[IMG_ROWS][IMG_COLS];

uint8_t video_buff[sizeof(PREAMBLE) + IMG_ROWS * IMG_COLS + sizeof(SUFFIX)];

uint8_t dma_flag = 0;
uint8_t button_flag = 0;

char msg[100];

// Your function definitions here
void pictureMode();
void videoMode();
void truncatedVideoMode();
void extraTruncatedVideoMode();
void compressedExtraTruncatedVideoMode();

uint8_t findNumberOfRepeatedPixels(uint8_t startRow, uint8_t startCol);
void transferRawBufferToIntermediate();
int rle(int bufferIndex);

int main(void)
{
  /* Reset of all peripherals */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_DCMI_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_I2C2_Init();
  MX_TIM1_Init();
  MX_TIM6_Init();

  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  ov7670_init();

    //*****Modes*****
	//pictureMode();
	//videoMode();
	//truncatedVideoMode();
	//extraTruncatedVideoMode();
	compressedExtraTruncatedVideoMode();
}

void pictureMode() {
	//Add pre-amble
	int bufferIndex= 0;
	for (int index = 0; index < sizeof(PREAMBLE); index++) {
		video_buff[bufferIndex++] = PREAMBLE[index];
	}
	//Add suffix
	bufferIndex= (IMG_ROWS * IMG_COLS/2)-1;
	for (int index = 0; index < sizeof(SUFFIX); index++) {
		video_buff[bufferIndex++] = SUFFIX[index];
	}
	//Loop forever
	while(1) {
		if (button_flag) {
			HAL_Delay(100);  //press delay

			HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);

			ov7670_snapshot(raw_buff);

			while(dma_flag == 0) {
				 HAL_Delay(0);
			}

			int bufferIndex = sizeof(PREAMBLE);
			for (int rawIndex = 1; rawIndex < 2*IMG_ROWS*IMG_COLS; rawIndex++) {
				if (rawIndex % 2 != 0) {
					video_buff[bufferIndex++] = raw_buff[rawIndex];
				}
			}

			//Export Data
			if (HAL_UART_GetState(&huart3) == HAL_UART_STATE_READY) {
				HAL_UART_Transmit_DMA(&huart3, (uint8_t *)video_buff, sizeof(PREAMBLE) + sizeof(SUFFIX) + IMG_ROWS*IMG_COLS);
			}

			while (HAL_UART_GetState(&huart3) != HAL_UART_STATE_READY) {
				HAL_Delay(0);
			}

			button_flag = 0;
			HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
		}
	}
}

void videoMode() {
	//Add pre-amble
	int bufferIndex= 0;
	for (int index = 0; index < sizeof(PREAMBLE); index++) {
		video_buff[bufferIndex++] = PREAMBLE[index];
	}
	//Add suffix
	bufferIndex= (IMG_ROWS * IMG_COLS/2)-1;
	for (int index = 0; index < sizeof(SUFFIX); index++) {
		video_buff[bufferIndex++] = SUFFIX[index];
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

		//Extract grey scale and place between pre-amble and suffix
		int bufferIndex = sizeof(PREAMBLE);
		for (int rawIndex = 1; rawIndex < 2*IMG_ROWS*IMG_COLS; rawIndex++) {
			if (rawIndex % 2 != 0) {
				video_buff[bufferIndex++] = raw_buff[rawIndex];
			}
		}

		//Export Data
		if (HAL_UART_GetState(&huart3) == HAL_UART_STATE_READY) {
			HAL_UART_Transmit_DMA(&huart3, (uint8_t *)video_buff, sizeof(PREAMBLE) + sizeof(SUFFIX) + IMG_ROWS*IMG_COLS);
		}
	}

}

void truncatedVideoMode() {
	//Add pre-amble
	int bufferIndex= 0;
	for (int index = 0; index < sizeof(PREAMBLE); index++) {
		video_buff[bufferIndex++] = PREAMBLE[index];
	}
	//Add suffix
	bufferIndex= (IMG_ROWS * IMG_COLS/2)-1;
	for (int index = 0; index < sizeof(SUFFIX); index++) {
		video_buff[bufferIndex++] = SUFFIX[index];
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

		//Truncate and place between pre-amble and suffix
		bufferIndex = sizeof(PREAMBLE);
		for (int rowIndex = 0; rowIndex < IMG_ROWS; rowIndex++) {
			for (int colIndex = 0; colIndex < IMG_COLS; colIndex++) {
				if (colIndex % 2 == 0) {
					video_buff[bufferIndex] = intermediate_buff[rowIndex][colIndex];
				} else {
					video_buff[bufferIndex] = video_buff[bufferIndex] | (intermediate_buff[rowIndex][colIndex] >> 4);
					bufferIndex++;
				}
			}
		}

		//Export Data
		if (HAL_UART_GetState(&huart3) == HAL_UART_STATE_READY) {
			HAL_UART_Transmit_DMA(&huart3, (uint8_t *)video_buff, sizeof(PREAMBLE) + IMG_ROWS * IMG_COLS/2 + sizeof(SUFFIX));
		}

	}

}

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


void compressedExtraTruncatedVideoMode() {
	uint8_t pass = 0;

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

		//Refresh frame
		int bufferIndex = 0;
		if (pass == 0) {

			//Add pre-amble
			for (int index = 0; index < sizeof(PREAMBLE); index++) {
				video_buff[bufferIndex++] = PREAMBLE[index];
			}

			//RLE and place between pre-amble and suffix
			bufferIndex = rle(sizeof(PREAMBLE));

			pass++;

		//Update difference
		} else {
			//Add delta
			for (int index = 0; index < sizeof(DELTA_PREAMBLE); index++) {
				video_buff[bufferIndex] = DELTA_PREAMBLE[index];
				bufferIndex++;
			}

			//Calculate difference and hot swap
			for (int rowIndex = 0; rowIndex < IMG_ROWS; rowIndex++) {
				for (int colIndex = 0; colIndex < IMG_COLS; colIndex++) {
					uint8_t difference = ((previous_buff[rowIndex][colIndex] >> 4) - (intermediate_buff[rowIndex][colIndex] >> 4)) & 0xF0;
					previous_buff[rowIndex][colIndex] = intermediate_buff[rowIndex][colIndex];
					intermediate_buff[rowIndex][colIndex] = difference;
				}
			}

			//Compress
			bufferIndex = rle(bufferIndex);

			pass++;

			if (pass == 6) pass = 0;
		}

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
