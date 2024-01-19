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
#include "main.h"
#include "config.h"
#include <stdio.h>


int8_t current_row = -1, current_col = -1;


int main(void)
{
  /* Reset of all peripherals. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  MX_GPIO_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();

  char zero_message[2];
  sprintf(zero_message, "0");
  char one_message[2];
  sprintf(one_message, "1");
  char two_message[2];
  sprintf(two_message, "2");
  char three_message[2];
  sprintf(three_message, "3");
  char four_message[2];
  sprintf(four_message, "4");
  char five_message[2];
  sprintf(five_message, "5");
  char six_message[2];
  sprintf(six_message, "6");
  char seven_message[2];
  sprintf(seven_message, "7");
  char eight_message[2];
  sprintf(eight_message, "8");
  char nine_message[2];
  sprintf(nine_message, "9");
  char a_message[2];
  sprintf(a_message, "a");
  char b_message[2];
  sprintf(b_message, "b");
  char c_message[2];
  sprintf(c_message, "c");
  char d_message[2];
  sprintf(d_message, "d");
  char e_message[2];
  sprintf(e_message, "e");
  char f_message[2];
  sprintf(f_message, "f");

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  
  /* Initialize ROW outputs */
  HAL_GPIO_WritePin(ROW0_GPIO_Port, ROW0_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(ROW1_GPIO_Port, ROW1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(ROW2_GPIO_Port, ROW2_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(ROW3_GPIO_Port, ROW3_Pin, GPIO_PIN_SET);

  /* Infinite loop */
  while (1)
  {

	 //Check row 0
	 HAL_GPIO_TogglePin(ROW0_GPIO_Port, ROW0_Pin);
	 HAL_GPIO_TogglePin(ROW0_GPIO_Port, ROW0_Pin);

	 if (current_col == 0) {
		 print_msg(three_message);
	 } else if (current_col == 1) {
		 print_msg(two_message);
	 } else if (current_col == 2) {
		 print_msg(one_message);
	 } else if (current_col == 3) {
		 print_msg(zero_message);
	 }

	 //Reset
	 current_col = -1;


	 //Check row 1
	 HAL_GPIO_TogglePin(ROW1_GPIO_Port, ROW1_Pin);
	 HAL_GPIO_TogglePin(ROW1_GPIO_Port, ROW1_Pin);

	 if (current_col == 0) {
		 print_msg(seven_message);
	 } else if (current_col == 1) {
		 print_msg(six_message);
	 } else if (current_col == 2) {
		 print_msg(five_message);
	 } else if (current_col == 3) {
		 print_msg(four_message);
	 }

	 //Reset
	 current_col = -1;


	 //Check row 2
	 HAL_GPIO_TogglePin(ROW2_GPIO_Port, ROW2_Pin);
	 HAL_GPIO_TogglePin(ROW2_GPIO_Port, ROW2_Pin);

	 if (current_col == 0) {
		 print_msg(b_message);
	 } else if (current_col == 1) {
		 print_msg(a_message);
	 } else if (current_col == 2) {
		 print_msg(nine_message);
	 } else if (current_col == 3) {
		 print_msg(eight_message);
	 }

	 //Reset
	 current_col = -1;


	 //Check row 3
	 HAL_GPIO_TogglePin(ROW3_GPIO_Port, ROW3_Pin);
	 HAL_GPIO_TogglePin(ROW3_GPIO_Port, ROW3_Pin);

	 if (current_col == 0) {
		 print_msg(f_message);
	 } else if (current_col == 1) {
		 print_msg(e_message);
	 } else if (current_col == 2) {
		 print_msg(d_message);
	 } else if (current_col == 3) {
		 print_msg(c_message);
	 }

	 //Reset
	 current_col = -1;

	 HAL_Delay(100);
  }
}


