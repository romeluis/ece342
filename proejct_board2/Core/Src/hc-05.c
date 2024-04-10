/*
 * hc-05.c
 *
 *  Created on: Apr 9, 2024
 *      Author: romeluis
 */

//Includes
#include "hc-05.h"
#include "main.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//Defines
#define OK_SIGNAL "OK"
#define SIZE_BUFFER_LENGTH 10
#define OK_SIGNAL_LENGTH sizeof("OK\r\n")
#define USART_TIMEOUT 1000
#define TIME_THRESHOLD 5000

//Enumerations
typedef enum {
	IDLE = 0,
	TRANSMITTING,
	RECEIVING,
	WAITING_CONFIRMATION,
	DISCONNECTED
} BluetoothState;

//Variables
BluetoothState moduleState = DISCONNECTED;

uint16_t dataSize = 0;
uint32_t currentTime = 0;
uint32_t previousTime = 0;

uint8_t sizeBuffer[SIZE_BUFFER_LENGTH];
uint8_t okBuffer[OK_SIGNAL_LENGTH] = "OK\r\n";

bool waitingForData = false;
bool waitingForSize = false;
bool waitingForOK = false;

bool sendingData = false;
bool sendingSize = false;
bool sendingOK = false;
bool exportingData = false;

UART_HandleTypeDef* BT_huart;
UART_HandleTypeDef* USB_huart;

void hc05Init(UART_HandleTypeDef* moduleHuart, UART_HandleTypeDef* sisterHuart) {
	BT_huart = moduleHuart;
	USB_huart = sisterHuart;
}

bool checkConnection() {
	if (HAL_GPIO_ReadPin(BT_STATE_GPIO_Port, BT_STATE_Pin)) {
		moduleState = IDLE;
		return true;
	} else {
		moduleState = DISCONNECTED;
		return false;
	}
}

bool cancelTransfer() {
	HAL_UART_Abort(BT_huart);
	return true;
}

bool checkTransfer() {
	if (BT_huart->gState == HAL_UART_STATE_READY || BT_huart->gState == HAL_UART_STATE_ERROR || BT_huart->gState == HAL_UART_STATE_TIMEOUT) {
		return false;
	}
	return true;
}

bool sendData(uint8_t* pdata, uint16_t size) {
	//Send data size
	sendingSize = true;
	sprintf((char*)sizeBuffer, "%d", size);
	HAL_UART_Transmit_DMA(BT_huart, sizeBuffer, SIZE_BUFFER_LENGTH);
	while(sendingSize) { //Wait until the DMA transfer completes
		HAL_Delay(0);
	}

	//Wait for confirmation signal
	waitingForOK = true;
	HAL_UART_Receive_DMA(BT_huart, okBuffer, OK_SIGNAL_LENGTH);
	while(waitingForOK) { //Wait until the DMA transfer completes
		HAL_Delay(0);
	}

	//Check confirmation
	if (okBuffer[0] != 'O' || okBuffer[1] != 'K') {
		while(1) {//Lock
			HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_SET);
		}
	}
	memset(okBuffer, 0, OK_SIGNAL_LENGTH);

	//Begin data transfer
	sendingData = true;
	HAL_UART_Transmit_DMA(BT_huart, pdata, size);
	previousTime = HAL_GetTick();
	while(sendingData) { //Wait until the DMA transfer completes
		currentTime = HAL_GetTick();
		if (currentTime - previousTime > TIME_THRESHOLD) { //Assume transfer complete given enough time
			cancelTransfer();
			waitingForData = false;
			break;
		}
		HAL_Delay(0);
	}

	return true; //Transfer complete
}

bool receiveData(uint8_t* pdata, uint16_t* psize) {
	//Start UART transfer to receive data size
	waitingForSize = true;
	HAL_UART_Receive_DMA(BT_huart, sizeBuffer, SIZE_BUFFER_LENGTH);
	while(waitingForSize) { //Wait until the DMA transfer completes
		HAL_Delay(0);
	}

	//Convert string to integer
	if (sizeBuffer[0] == 0) { //Sometimes the first character is zero so the number must be shifted
		shiftSizeBuffer();
	}
	dataSize = atoi((char*)sizeBuffer);
	psize = &dataSize;

	//Send confirmation signal
	HAL_UART_Transmit(BT_huart, okBuffer, OK_SIGNAL_LENGTH, USART_TIMEOUT);

	//Begin data transfer
	waitingForData = true;
	HAL_UART_Receive_DMA(BT_huart, pdata, dataSize);
	previousTime = HAL_GetTick();
	while(waitingForData) { //Wait until the DMA transfer completes
		currentTime = HAL_GetTick();
		if (currentTime - previousTime > TIME_THRESHOLD) { //Assume transfer complete given enough time
			cancelTransfer();
			waitingForData = false;
			break;
		}
		HAL_Delay(0);
	}

	return true; //Transfer complete
}

void requestNextFrame() {
	//Send confirmation signal
	sendingOK = true;
	HAL_UART_Transmit_DMA(BT_huart, okBuffer, OK_SIGNAL_LENGTH);
	while(sendingOK) {
		HAL_Delay(0);
	}
}

void waitForNextFrameRequest() {
	//Wait for confirmation signal
	waitingForOK = true;
	HAL_UART_Receive_DMA(BT_huart, okBuffer, OK_SIGNAL_LENGTH);
	while(waitingForOK) { //Wait until the DMA transfer completes
		HAL_Delay(0);
	}

	//Check confirmation
	if (okBuffer[0] != 'O' || okBuffer[1] != 'K') {
		while(1) {//Lock
			HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_SET);
		}
	}
	memset(okBuffer, 0, OK_SIGNAL_LENGTH);
}

void shiftSizeBuffer() {
	sizeBuffer[0] = sizeBuffer[1];
	sizeBuffer[1] = sizeBuffer[2];
	sizeBuffer[2] = sizeBuffer[3];
	sizeBuffer[3] = sizeBuffer[4];
	sizeBuffer[4] = sizeBuffer[5];
	sizeBuffer[5] = sizeBuffer[6];
	sizeBuffer[6] = sizeBuffer[7];
	sizeBuffer[7] = sizeBuffer[8];
	sizeBuffer[8] = sizeBuffer[9];
	sizeBuffer[9] = 0;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart != BT_huart) {
		return;
	}

	if (waitingForSize) {
		waitingForSize = false;
		return;
	}

	if (waitingForData) {
		waitingForData = false;
		return;
	}

	if (waitingForOK) {
		waitingForOK = false;
		return;
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart != BT_huart) {
		return;
	}

	if (sendingSize) {
		sendingSize = false;
		return;
	}

	if (sendingData) {
		sendingData = false;
		return;
	}

	if (sendingOK) {
		sendingOK = false;
		return;
	}

	if (exportingData) {
		exportingData = false;
		return;
	}
}
