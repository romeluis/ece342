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
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "config.h"
#include "fixedpt.h"

//Constants
const uint16_t AMPLITUDE = 512;
const int OFFSET = AMPLITUDE + 100;
const double PI = 3.14;
const double SIN_STEP = (PI/50.0);
const int SIN_RESOLUTION = (2*PI)/(SIN_STEP);

const int FOURIER_UPPERBOUND = 6;
const int LOWER_FOURIER_UPPERBOUND = FOURIER_UPPERBOUND;

//Globals
float* sinValues;
fixedpt* sinValuesFixed;
float* squareValues;
float* triangleValues;
float* sawValues;
char message[100];

//Function declarations
void outputSinViaMath();
void precalculateSin();
void precalculateSinFixed();
void outputSinViaLUT();
void outputSinViaLUTFixed();

double square(int upperBound, double radians);
fixedpt square_fixed(int upperBound, double radians);
void outputSquareViaMath();
void outputSquareViaMathFixed();
void precalculateSquare();
void outputSquareViaLUT();

double triangle(int upperBound, double radians);
void outputTriangleViaMath();
void precalculateTriangle();
void outputTriangleViaLUT();

double saw(int upperBound, double radians);
void outputSawViaMath();
void precalculateSaw();
void outputSawViaLUT();

void printMessage(char* preface, double value);


int main(void)
{
  /* Reset of all peripherals. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DAC_Init();
  MX_TIM6_Init();
  MX_USART3_Init();
  MX_DAC_Init();

  char message[100];
  
  // DAC SETUP
  HAL_DAC_Init(&hdac);
	HAL_DAC_Start(&hdac, DAC_CHANNEL_1);

	
	//LAB SETUP
	//precalculateSin();
	//precalculateSinFixed();
	//precalculateSquare();
	//precalculateTriangle();
	//precalculateSaw();
	
	//PART 1
	 //int timeStart = HAL_TIM_Base_Start(&htim6);
	 
	 //outputSinViaLUTFixed();

   //int timeEnd = __HAL_TIM_GET_COUNTER(&htim6);

   //printMessage("Outputting took: ", timeEnd - timeStart);
	
  while (1)
  {
		//outputSinViaMath();
		//outputSinViaLUT();
		//outputSinViaLUTFixed();
		
		//outputSquareViaMath();
		//outputSquareViaMathFixed();
		//outputSquareViaLUT();
		
		 //outputTriangleViaMath();
		 //outputTriangleViaLUT();
		
		 outputSawViaMath();
		 //outputSawViaLUT();
  }
 }

//Functions
 
//********SIN*********
void outputSinViaMath() {
	for (double radians = 2*PI; radians <= 4*PI; radians += 0.01) {
		double output = AMPLITUDE * sin(radians) + OFFSET;
		HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, output);
	}
}

void outputSinViaLUT() {
	for (int index = 0; index < SIN_RESOLUTION; index++) {
		HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, sinValues[index]);
	}
}

void outputSinViaLUTFixed() {
	for (int index = 0; index < SIN_RESOLUTION; index++) {
		HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R,AMPLITUDE * FXD_TO_FLOAT(sinValuesFixed[index] + OFFSET));
	}
}

void precalculateSin() {
	sinValues = malloc(sizeof(float) * SIN_RESOLUTION);

	int index = 0;
	for (double radians = 2*PI; radians <= 4*PI; radians += SIN_STEP) {
		double output = AMPLITUDE * sin(radians) + OFFSET;
		sinValues[index] = output;
		index++;
	}
}

void precalculateSinFixed() {
	sinValuesFixed = malloc(sizeof(float) * SIN_RESOLUTION);
	int index = 0;
	for (double radians = 2*PI; radians <= 4*PI; radians += SIN_STEP) {
		double output = sin(radians)+1;
		sinValuesFixed[index] = FXD_FROM_FLOAT(output);
		index++;
	}
}

//********SQUARE*********
double square(int upperBound, double radians) {
	double solution = 0;
	
	for(double n = 0; n <= upperBound; n += 1.0) {
		solution += (1.0/(double)(2*n+1)) * sin((2*n + 1) * radians);
	}
	
	return 0.5 + (4.0/(PI)) * solution;
}

fixedpt square_fixed(int upperBound, double radians) {
	fixedpt solution = 0;
	
	for(double n = 0; n <= upperBound; n += 1.0) {
		solution = FXD_ADD(solution, FXD_MUL(FXD_FROM_FLOAT(1.0/(double)(2*n+1)), FXD_FROM_FLOAT(sin((2*n + 1) * radians))));
	}
	//printMessage("Solution: ", FXD_TO_FLOAT(FXD_ADD(0.5, FXD_MUL((4.0/(PI)), solution))));
	return FXD_ADD(FXD_FROM_FLOAT(0.5), FXD_MUL(FXD_FROM_FLOAT(4.0/PI), solution));
}


void outputSquareViaMath() {
	for (double radians = 2*PI; radians <= 4*PI; radians += 0.01) {
		double output = AMPLITUDE * square(FOURIER_UPPERBOUND, radians) + OFFSET;
		HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, output);
	}
}

void outputSquareViaMathFixed() {
	for (double radians = 2*PI; radians <= 4*PI; radians += 0.01) {
		//fixedpt output = FXD_ADD(FXD_MUL(AMPLITUDE, square_fixed(LOWER_FOURIER_UPPERBOUND, radians)), OFFSET);
		float output = AMPLITUDE * FXD_TO_FLOAT(square_fixed(LOWER_FOURIER_UPPERBOUND, radians)) + OFFSET;
		//printMessage("Output: ", FXD_TO_FLOAT(square_fixed(LOWER_FOURIER_UPPERBOUND, radians)));
		HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, output);
	}
}

void precalculateSquare() {
	squareValues = malloc(sizeof(float) * SIN_RESOLUTION);
	int index = 0;
	for (double radians = 2*PI; radians <= 4*PI; radians += SIN_STEP) {
		double output = AMPLITUDE * square(FOURIER_UPPERBOUND, radians) + OFFSET;
		squareValues[index] = output;
		index++;
	}
}

void outputSquareViaLUT() {
	for (int index = 0; index < SIN_RESOLUTION; index++) {
		HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, squareValues[index]);
	}
}

//********TRIANGLE*********
double triangle(int upperBound, double radians) {
	double solution = 0;
	
	for(int n = 0; n <= upperBound; n++) {
		solution += ((pow(-1, n))/pow(2*n+1, 2)) * sin((2*n + 1) * radians);
	}
	
	return 0.5 + (4.0/(PI*PI)) * solution;
}

void outputTriangleViaMath() {
	for (double radians = 2*PI; radians <= 4*PI; radians += 0.01) {
		double output = AMPLITUDE * triangle(FOURIER_UPPERBOUND, radians) + OFFSET;
		HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, output);
	}
}

void precalculateTriangle() {
	triangleValues = malloc(sizeof(float) * SIN_RESOLUTION);
	int index = 0;
	for (double radians = 2*PI; radians <= 4*PI; radians += SIN_STEP) {
		double output = AMPLITUDE * triangle(FOURIER_UPPERBOUND, radians) + OFFSET;
		triangleValues[index] = output;
		index++;
	}
}

void outputTriangleViaLUT() {
	for (int index = 0; index < SIN_RESOLUTION; index++) {
		HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, triangleValues[index]);
	}
}

//********SAW*********
double saw(int upperBound, double radians) {
	double solution = 0.0;
	
	for(double n = 1.0; n <= upperBound; n += 1.0) {
		solution += (1.0/n) * sin(n*radians);
	}
	
	return 0.5 - (1.0/(double)PI) * solution;
}

void outputSawViaMath() {
	for (double radians = 2*PI; radians <= 4*PI; radians += 0.01) {
		double output = AMPLITUDE * saw(FOURIER_UPPERBOUND, radians) + OFFSET;
		HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, output);
	}
}

void precalculateSaw() {
	sawValues = malloc(sizeof(float) * SIN_RESOLUTION);
	int index = 0;
	for (double radians = 2*PI; radians <= 4*PI; radians += SIN_STEP) {
		double output = AMPLITUDE * saw(FOURIER_UPPERBOUND, radians) + OFFSET;
		sawValues[index] = output;
		index++;
	}
}

void outputSawViaLUT() {
	for (int index = 0; index < SIN_RESOLUTION; index++) {
		HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, sawValues[index]);
	}
}

//********PRINT*********
void printMessage(char* preface, double value) {
	sprintf(message, "%s%f\r\n", preface, value);
	print_msg(message);
}