/**
 ******************************************************************************
 * @file    ds3231.h
 * @author  Pablo Fuentes
 * @version V1.0.1
 * @date    2019
 * @brief   Header de DS3231 Library
 ******************************************************************************
 */

#ifndef __DS3231_H
#define __DS3231_H

#include "eonOS.h"

/**
 ===============================================================================
              ##### Public functions #####
 ===============================================================================
 */

// Init
void ds3231_init(I2C_TypeDef *I2Cx);

// Check Status register
bool ds3231_clearStatusReg(void);
uint8_t ds3231_readStatusRegister(void);

// Adjust time
bool ds3231_adjust(uint8_t d, uint8_t mt, uint16_t y, uint8_t h, uint8_t m, uint8_t s);
bool ds3231_adjustUnix(uint32_t unix);

// Get current time
bool ds3231_now(LocalTime_t *_lt);
uint32_t ds3231_nowUnix(void);

// Alarm 1
bool ds3231_setAlarm1(uint32_t unix); // Alarm when hours, minutes, and seconds match
bool ds3231_checkAlarm1isOK(uint32_t unix);
bool ds3231_setAlarm1Forced(uint32_t unix); // Alarm when hours, minutes, and seconds match. This function try 10 times to set the alarm

// Alarm 2
bool ds3231_setAlarm2(uint32_t unix); // Alarm when hours and minutes match
bool ds3231_checkAlarm2isOK(uint32_t unix);
bool ds3231_setAlarm2Forced(uint32_t unix);

// Print Debug
void ds3231_printTime(LocalTime_t *_lt);
void ds3231_printUnix(uint32_t unix);

#endif