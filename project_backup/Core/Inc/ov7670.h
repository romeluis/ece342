#ifndef __OV7670_H
#define __OV7670_H

#include "main.h"

#define ADDR_OV7670_READ (uint16_t)(0x43)
#define ADDR_OV7670_WRITE (uint16_t)(0x42)

// #define OV7670_REG_NUM 123
#define OV7670_REG_NUM 122

// Image settings
#define IMG_ROWS 144
#define IMG_COLS 174

uint8_t ov7670_read(uint8_t reg);
HAL_StatusTypeDef ov7670_write(uint8_t reg,uint8_t val);

uint8_t ov7670_init(void);

void ov7670_snapshot(uint8_t *buff);
void ov7670_capture(uint8_t *buff);

#endif
