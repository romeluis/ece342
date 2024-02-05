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

#include "dfr0151.h"
#include "config.h"
#include "stm32f4xx_it.h"

#include <stdio.h>
#include <string.h>

uint8_t alarmStatus;

int main(void)
{
  /* Reset of all peripherals. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();  
  rtc_init();

  char msg[100];

  //Get time & print
  uint8_t hour = 10;
  uint8_t minute = 0;
  uint8_t second = 0;
  uint8_t weekday = 2;
  uint8_t day = 1;
  uint8_t month = 7;
  uint8_t year = 24;

  rtc_set_date(weekday, day, month, year);
  rtc_set_time(hour, minute, second);

  second = eeprom_read(0);
  minute = eeprom_read(1);
  hour = eeprom_read(2);
  weekday = eeprom_read(3);
  day = eeprom_read(4);
  month = eeprom_read(5);
  year = eeprom_read(6);
  alarmStatus = eeprom_read(7);

  sprintf(msg, "The saved date is: weekday %d, day %d, month %d, year 20%d, at %d:%d:%d\r\n", weekday, day, month, year, hour, minute, second);
  print_msg(msg);

  hour = 0;
  minute = 0;
  second = 0;
  weekday = 1;
  day = 1;
  month = 1;
  year = 0;

  rtc_get_date(&weekday, &day, &month, &year);
  rtc_get_time(&hour, &minute, &second);

  sprintf(msg, "It is: weekday %d, day %d, month %d, year 20%d, at %d:%d:%d\r\n", weekday, day, month, year, hour, minute, second);
  print_msg(msg);

  while (1)
  {
	  rtc_get_date(&weekday, &day, &month, &year);
	  rtc_get_time(&hour, &minute, &second);

	  char dayS[10];
	  if (weekday == 1) {
		  sprintf(dayS, "Sunday");
	  } else if (weekday == 2) {
		  sprintf(dayS, "Monday");
	  } else if (weekday == 3) {
		  sprintf(dayS, "Tuesday");
	  } else if (weekday == 4) {
		  sprintf(dayS, "Wednesday");
	  } else if (weekday == 5) {
		  sprintf(dayS, "Thursday");
	  } else if (weekday == 6) {
		  sprintf(dayS, "Friday");
	  } else if (weekday == 7) {
		  sprintf(dayS, "Saturday");
	  }

	  char monthS[10];
	  if (month == 1) {
		  sprintf(monthS, "January");
	  } else if (month == 2) {
		  sprintf(monthS, "February");
	  } else if (month == 3) {
		  sprintf(monthS, "March");
	  } else if (month == 4) {
		  sprintf(monthS, "April");
	  } else if (month == 5) {
		  sprintf(monthS, "May");
	  } else if (month == 6) {
		  sprintf(monthS, "June");
	  } else if (month == 7) {
		  sprintf(monthS, "July");
	  } else if (month == 8) {
		  sprintf(monthS, "August");
	  } else if (month == 9) {
		  sprintf(monthS, "September");
	  } else if (month == 10) {
		  sprintf(monthS, "October");
	  } else if (month == 11) {
		  sprintf(monthS, "November");
	  } else if (month == 12) {
		  sprintf(monthS, "December");
	  }

	  sprintf(msg, "%s %s %dst, 20%d at %d:%d:%d. Alarm Status: %d\r\n", dayS, monthS, day, year, hour, minute, second, alarmStatus);
	  print_msg(msg);

	  if(interruptStatus) {
		  if (!alarmStatus) {
			  print_msg("Time reset. Previous time saved. Alarm active.\r\n");
			  reset();
			  alarmStatus = 1;
			  eeprom_write(7, &alarmStatus, 1);
			  interruptStatus = 0;
		  } else {
			  print_msg("Time reset. Previous time saved. Alarm disabled.\r\n");
			  reset();
			  alarmStatus = 0;
			  eeprom_write(7, &alarmStatus, 1);
			  interruptStatus = 0;
		  }
	  }

	  if (alarmStatus && hour == 10 && minute == 0 && second == 10) {
		  while(!interruptStatus) {
			  HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
			  HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
			  HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
			  HAL_Delay(100);
		  }
	  }

	  HAL_Delay(1000);
  }
}

void reset() {
	  uint8_t hourI = 0;
	  uint8_t minuteI = 0;
	  uint8_t secondI = 0;
	  uint8_t weekdayI = 0;
	  uint8_t dayI = 0;
	  uint8_t monthI = 0;
	  uint8_t yearI = 0;

	 rtc_get_date(&weekdayI, &dayI, &monthI, &yearI);
	 rtc_get_time(&hourI, &minuteI, &secondI);

	 eeprom_write(0, &secondI, 1);
	 eeprom_write(1, &minuteI, 1);
	 eeprom_write(2, &hourI, 1);
	 eeprom_write(3, &weekdayI, 1);
	 eeprom_write(4, &dayI, 1);
	 eeprom_write(5, &monthI, 1);
	 eeprom_write(6, &yearI, 1);

	//Reset simulation
	 hourI = 10;
	 minuteI = 0;
	 secondI = 0;
	 weekdayI = 2;
	 dayI = 1;
	 monthI = 7;
	 yearI = 24;

	rtc_set_date(weekdayI, dayI, monthI, yearI);
	rtc_set_time(hourI, minuteI, secondI);
}
