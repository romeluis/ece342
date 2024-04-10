/*
 * hc-05.h
 *
 *  Created on: Apr 9, 2024
 *      Author: romeluis
 */

#ifndef INC_HC_05_H_
#define INC_HC_05_H_

#include <stdbool.h>
#include "main.h"

extern bool exportingData;

void hc05Init(UART_HandleTypeDef* moduleHuart, UART_HandleTypeDef* sisterHuart);

bool checkConnection();
bool cancelTransfer();
bool checkTransfer();

bool sendData(uint8_t* pdata, uint16_t size);
bool receiveData(uint8_t* pdata, uint16_t* psize);

void shiftSizeBuffer();

void waitForNextFrameRequest();
void requestNextFrame();

#endif /* INC_HC_05_H_ */
