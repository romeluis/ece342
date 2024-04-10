#include "dfr0151.h"

extern I2C_HandleTypeDef hi2c1;

int interruptStatus = 0;

uint8_t bcd2bin(uint8_t data){
 return ((data >> 4) * 10) + (data & 0x0F);
}

uint8_t bin2bcd(uint8_t data){
  return ((data / 10) << 4)|(data % 10);
}

uint8_t rtc_read(uint8_t address)
{
  uint8_t data;

  if (HAL_I2C_Mem_Read(&hi2c1,ADDR_DS1307,address,I2C_MEMADD_SIZE_8BIT,&data,1,100) != HAL_OK) {
    Error_Handler();
  }

  return data;
}

void rtc_write(uint8_t address,uint8_t data)
{
  if (HAL_I2C_Mem_Write(&hi2c1,ADDR_DS1307,address,I2C_MEMADD_SIZE_8BIT,&data,1,100) != HAL_OK){
    Error_Handler();
  }
}

void rtc_init()
{
  // Initialize Real-Time Clock peripheral
  uint8_t rs=0, sqwe=1, out=0;
  
  rs&=3;
  if (sqwe) rs|=0x10;
  if (out) rs|=0x80;

  rtc_write(0x07,rs);
}

void rtc_get_time(uint8_t *hour,uint8_t *min,uint8_t *sec)
{
  *hour = bcd2bin(rtc_read(2));
  *min = bcd2bin(rtc_read(1));
  *sec = bcd2bin(rtc_read(0));
  return;
}

void rtc_set_time(uint8_t hour,uint8_t min,uint8_t sec)
{
  rtc_write(2, bin2bcd(hour));
  rtc_write(1, bin2bcd(min));
  rtc_write(0, bin2bcd(sec));
  return;
}

void rtc_get_date(uint8_t *week_day,uint8_t *day,uint8_t *month,uint8_t *year)
{
  *week_day = bcd2bin(rtc_read(3));
  *day = bcd2bin(rtc_read(4));
  *month = bcd2bin(rtc_read(5));
  *year = bcd2bin(rtc_read(6));
  return;
}

void rtc_set_date(uint8_t week_day,uint8_t day,uint8_t month,uint8_t year)
{
	rtc_write(3, bin2bcd(week_day));
	rtc_write(4, bin2bcd(day));
	rtc_write(5, bin2bcd(month));
	rtc_write(6, bin2bcd(year));
	return;
}

void eeprom_write(uint16_t address, uint8_t *data, uint16_t size) {
	if (HAL_I2C_Mem_Write(&hi2c1,ADDR_EEPROM,address,I2C_MEMADD_SIZE_16BIT,data,size,100) != HAL_OK){
	    Error_Handler();
	  }
	HAL_Delay(11);
}

uint8_t eeprom_read(uint16_t address) {
	uint8_t data;

	 if (HAL_I2C_Mem_Read(&hi2c1,ADDR_EEPROM,address,I2C_MEMADD_SIZE_16BIT,&data,1,100) != HAL_OK) {
	    Error_Handler();
	  }
	 HAL_Delay(11);

	 return data;
}
