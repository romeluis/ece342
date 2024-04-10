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
#include "main.h"
#include "config.h"

#include <stdio.h>
#include <math.h>

const double FREQ = 120;
const int AMP = 5;
const int OFFSET = 10;
const double MAX_STEP = (1.0/FREQ);
const double INTERVAL = MAX_STEP/5.0;

int calculateSin(double step) {
	return AMP*sin(FREQ*(3.14/180.0)*step) + OFFSET;
}

int main(void)
{
  /* Reset of all peripherals. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC3_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_DAC_Init();

  uint16_t adc_res, mask = 0xff00;
  char message[100];

  //  ADC example
  //HAL_ADC_Start(&hadc3);
  //HAL_ADC_PollForConversion(&hadc3, 100);
  //adc_res = HAL_ADC_GetValue(&hadc3);
  //sprintf(message, "adc_res=%d\r\n", adc_res);
  //print_msg(message);

  // DAC example
  HAL_DAC_Init(&hdac);
  //HAL_DAC_Start(&hdac, DAC_CHANNEL_1);

  while (1)
  {
	  HAL_ADC_Start(&hadc3);
	  HAL_ADC_PollForConversion(&hadc3, 100);
	  adc_res = HAL_ADC_GetValue(&hadc3);
	  //sprintf(message, "adc_res=%d\r\n", adc_res);
	  //print_msg(message);
	  /* Part 2
	  */
//	  double step = 0;
	  //while (step <= MAX_STEP) {
	  HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, adc_res);
//		  step += INTERVAL;
	  //}
  }
 }
