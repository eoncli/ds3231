/**
  ******************************************************************************
  * @file    ds3231.c 
  * @author  Pablo Fuentes
	* @version V1.0.1
  * @date    2019
  * @brief   DS3231 Functions
  ******************************************************************************
*/

#include "ds3231.h"

/** 
 ===============================================================================
              ##### Macro definitions #####
 ===============================================================================
 */

#define DS3231_ADDR 0xD0 // 0x68 << 1

typedef struct
{
  I2C_TypeDef *I2Cx;
} ds3231_t;

static ds3231_t ds;
static char tx_i2c_data[20];
static char rx_i2c_data[20];
LocalTime_t _lt;

/** 
 ===============================================================================
              ##### Private functions #####
 ===============================================================================
 */

static uint8_t bcd2bin(uint8_t val) { return val - 6 * (val >> 4); }
static uint8_t bin2bcd(uint8_t val) { return val + 6 * (val / 10); }

/** 
 ===============================================================================
              ##### Public functions #####
 ===============================================================================
 */

void ds3231_init(I2C_TypeDef *I2Cx) {
  ds.I2Cx = I2Cx;
}

bool ds3231_clearStatusReg(void) {
  tx_i2c_data[0] = 0x0F; // Status register
  tx_i2c_data[1] = 0x00; // Clear all Status Register
  if (i2c_write(ds.I2Cx, DS3231_ADDR, tx_i2c_data, 2, I2C_STOP) == 2) return true;
  return false;
}

uint8_t ds3231_readStatusRegister(void) {
  tx_i2c_data[0] = 0x0F;                                                                     // Status register
  if (i2c_write(ds.I2Cx, DS3231_ADDR, tx_i2c_data, 1, I2C_STOP) != 1) return (uint8_t) 0xFF; //error
  i2c_read(ds.I2Cx, DS3231_ADDR, rx_i2c_data, 1, I2C_STOP);                                  // read control register to avoid change any other configuration
  return (uint8_t) rx_i2c_data[0];
}

bool ds3231_adjust(uint8_t d, uint8_t mt, uint16_t y, uint8_t h, uint8_t m, uint8_t s) {
  tx_i2c_data[0] = 0x00;
  tx_i2c_data[1] = bin2bcd(s) & 0x7F;
  tx_i2c_data[2] = bin2bcd(m);
  tx_i2c_data[3] = bin2bcd(h) & 0x3F;
  tx_i2c_data[4] = 0;
  tx_i2c_data[5] = bin2bcd(d);
  tx_i2c_data[6] = bin2bcd(mt);
  tx_i2c_data[7] = bin2bcd(y - 2000);
  if (i2c_write(ds.I2Cx, DS3231_ADDR, tx_i2c_data, 8, I2C_STOP) == 8)
    return true;
  else
    return false;
}

bool ds3231_adjustUnix(uint32_t unix) {
  unix2time(unix, &_lt);
  tx_i2c_data[0] = 0x00;
  tx_i2c_data[1] = bin2bcd(_lt.seconds) & 0x7F;
  tx_i2c_data[2] = bin2bcd(_lt.minutes);
  tx_i2c_data[3] = bin2bcd(_lt.hours) & 0x3F;
  tx_i2c_data[4] = 0;
  tx_i2c_data[5] = bin2bcd(_lt.day);
  tx_i2c_data[6] = bin2bcd(_lt.month);
  tx_i2c_data[7] = bin2bcd(_lt.year);
  if (i2c_write(ds.I2Cx, DS3231_ADDR, tx_i2c_data, 8, I2C_STOP) == 8)
    return true;
  else
    return false;
}

bool ds3231_now(LocalTime_t *lt) {
  tx_i2c_data[0] = 0;
  if (i2c_write(ds.I2Cx, DS3231_ADDR, tx_i2c_data, 1, I2C_STOP) != 1) return false;

  i2c_read(ds.I2Cx, DS3231_ADDR, rx_i2c_data, 7, I2C_STOP);
  lt->day = bcd2bin(rx_i2c_data[4]);
  lt->month = bcd2bin(rx_i2c_data[5]);
  lt->year = bcd2bin(rx_i2c_data[6]);
  lt->hours = bcd2bin(rx_i2c_data[2]);
  lt->minutes = bcd2bin(rx_i2c_data[1]);
  lt->seconds = bcd2bin(rx_i2c_data[0] & 0x7F);

  return true;
}

uint32_t ds3231_nowUnix(void) {
  uint32_t result = 0;
  int8_t tries = 10;
  tx_i2c_data[0] = 0;

  do {
    if (i2c_write(ds.I2Cx, DS3231_ADDR, tx_i2c_data, 1, I2C_STOP) != 1) continue;
    i2c_read(ds.I2Cx, DS3231_ADDR, rx_i2c_data, 7, I2C_STOP);
    result = time2unix(bcd2bin(rx_i2c_data[4]), bcd2bin(rx_i2c_data[5]), bcd2bin(rx_i2c_data[6]),
                       bcd2bin(rx_i2c_data[2]), bcd2bin(rx_i2c_data[1]), bcd2bin(rx_i2c_data[0] & 0x7F));
  } while (result == 0 && tries--);

  return result;
}

bool ds3231_setAlarm1(uint32_t unix) {
  unix2time(unix, &_lt);
  tx_i2c_data[0] = 0x07;
  i2c_write(ds.I2Cx, DS3231_ADDR, tx_i2c_data, 1, I2C_STOP);
  i2c_read(ds.I2Cx, DS3231_ADDR, rx_i2c_data, 9, I2C_STOP);
  // Configuring for "Alarm when hours, minutes, and seconds match"
  tx_i2c_data[0] = 0x07;                        // Alarm1 register
  tx_i2c_data[1] = bin2bcd(_lt.seconds) & 0x7F; // A1M1 = 0
  tx_i2c_data[2] = bin2bcd(_lt.minutes) & 0x7F; // A1M2 = 0
  tx_i2c_data[3] = bin2bcd(_lt.hours) & 0x3F;   // A1M3 = 0
  tx_i2c_data[4] = 0x80;                        //A1M4 = 1
  tx_i2c_data[5] = rx_i2c_data[4];              // Alarm 2 register
  tx_i2c_data[6] = rx_i2c_data[5];              // Alarm 2 register
  tx_i2c_data[7] = rx_i2c_data[6];              // Alarm 2 register
  tx_i2c_data[8] = rx_i2c_data[7] | 0x05;       // Enable Alarm 1 Interrupt
  tx_i2c_data[9] = rx_i2c_data[8] & 0xFE;       // Clear Alarm 1 Interrupt
  if (i2c_write(ds.I2Cx, DS3231_ADDR, tx_i2c_data, 10, I2C_STOP) != 10) return false;
  return true;
}

bool ds3231_checkAlarm1isOK(uint32_t unix) {
  unix2time(unix, &_lt);
  tx_i2c_data[0] = 0x07;
  i2c_write(ds.I2Cx, DS3231_ADDR, tx_i2c_data, 1, I2C_STOP);
  i2c_read(ds.I2Cx, DS3231_ADDR, rx_i2c_data, 9, I2C_STOP);
  // check overall alarm settings
  if ((bcd2bin(rx_i2c_data[0] & 0x7F) != _lt.seconds) && ((rx_i2c_data[0] & 0x80) != 0x00)) return false;
  if ((bcd2bin(rx_i2c_data[1] & 0x7F) != _lt.minutes) && ((rx_i2c_data[1] & 0x80) != 0x00)) return false;
  if ((bcd2bin(rx_i2c_data[2] & 0x3F) != _lt.hours) && ((rx_i2c_data[2] & 0x80) != 0x00)) return false;
  if ((rx_i2c_data[3] & 0x80) != 0x80) return false;
  if ((rx_i2c_data[7] & 0x05) != 0x05) return false; // check if alarm interrupt is enable
  if ((rx_i2c_data[8] & 0x01) != 0x00) return false; // check if alarm flag is cleared
  return true;
}

bool ds3231_setAlarm1Forced(uint32_t unix) {
  uint8_t rt;
  for (rt = 0; rt < 10; rt++) {
    if (ds3231_setAlarm1(unix) == false) continue;
    if (ds3231_checkAlarm1isOK(unix) == true) return true;
    delay(1);
  }
  return false;
}

bool ds3231_setAlarm2(uint32_t unix) {
  unix2time(unix, &_lt);
  tx_i2c_data[0] = 0x0B;
  i2c_write(ds.I2Cx, DS3231_ADDR, tx_i2c_data, 1, I2C_STOP);
  i2c_read(ds.I2Cx, DS3231_ADDR, rx_i2c_data, 5, I2C_STOP);
  // Configuring for "Alarm when hours and minutes match"
  tx_i2c_data[0] = 0x0B;                        // Alarm2 register
  tx_i2c_data[1] = bin2bcd(_lt.minutes) & 0x7F; // A2M2 = 0
  tx_i2c_data[2] = bin2bcd(_lt.hours) & 0x3F;   // A2M3 = 0
  tx_i2c_data[3] = 0x80;                        //A2M4 = 1
  tx_i2c_data[4] = rx_i2c_data[3] | 0x06;       // Enable Alarm 2 Interrupt
  tx_i2c_data[5] = rx_i2c_data[4] & 0xFD;       // Clear Alarm 2 Interrupt
  if (i2c_write(ds.I2Cx, DS3231_ADDR, tx_i2c_data, 6, I2C_STOP) != 6) return false;
  return true;
}

bool ds3231_checkAlarm2isOK(uint32_t unix) {
  unix2time(unix, &_lt);
  tx_i2c_data[0] = 0x0B;
  i2c_write(ds.I2Cx, DS3231_ADDR, tx_i2c_data, 1, I2C_STOP);
  i2c_read(ds.I2Cx, DS3231_ADDR, rx_i2c_data, 5, I2C_STOP);
  // check overall alarm settings
  if ((bcd2bin(rx_i2c_data[0] & 0x7F) != _lt.minutes) && ((rx_i2c_data[0] & 0x80) != 0x00)) return false;
  if ((bcd2bin(rx_i2c_data[1] & 0x3F) != _lt.hours) && ((rx_i2c_data[1] & 0x80) != 0x00)) return false;
  if ((rx_i2c_data[2] & 0x80) != 0x00) return false;
  if ((rx_i2c_data[3] & 0x06) != 0x06) return false; // check if alarm interrupt is enable
  if ((rx_i2c_data[4] & 0x02) != 0x00) return false; // check if alarm flag is cleared
  return true;
}

bool ds3231_setAlarm2Forced(uint32_t unix) {
  uint8_t rt;
  for (rt = 0; rt < 10; rt++) {
    if (ds3231_setAlarm2(unix) == false) continue;
    if (ds3231_checkAlarm2isOK(unix) == true) return true;
    delay(1);
  }
  return false;
}

void ds3231_printTime(LocalTime_t *lt) {
  lprint("{D}/{D}/{D} {D}:{D}:{D}\n", lt->day,
         lt->month,
         lt->year,
         lt->hours,
         lt->minutes,
         lt->seconds);
}

void ds3231_printUnix(uint32_t unix) {
  unix2time(unix, &_lt);
  lprint("{D}/{D}/{D} {D}:{D}:{D}\n", _lt.day,
         _lt.month,
         _lt.year,
         _lt.hours,
         _lt.minutes,
         _lt.seconds);
}