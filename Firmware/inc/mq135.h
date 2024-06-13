#ifndef _MYMQ135_H
#define _MYMQ135_H

#include "simcom_api.h"
#include <stdio.h>
#include <stdint.h>
/// The load resistance on the board

#define RLOAD 10.0

/// Calibration resistance at atmospheric CO2 level

#define RZERO 76.63
//#define RZERO  6125 //4973 5672 4668.26 5292.8 6619.89 9937.62  8031.71 7223.11
/// Parameters for calculating ppm of CO2 from sensor resistance

#define PARA 116.6020682
#define PARB 2.769034857

//#define PARA 780.25597 // for LPG ---MQ-2
//#define PARB 1.634397  // for LPC ---MQ-2
//#define PARA 115.3653279 // for CO ---MQ-2
//#define PARB 2.772544  // for CO ---MQ-2
//#define PARA 1841.304171 // for Smoke ---MQ-2
//#define PARB 2.665252  // for Smoke ---MQ-2

/// Parameters to model temperature and humidity dependence
#define CORA 0.00035
#define CORB 0.02718
#define CORC 1.39538
#define CORD 0.0018

/// Atmospheric CO2 level for calibration purposes
#define ATMOCO2 400 //397.13



void MQ135_init(void);

// Hàm lấy hệ số hiệu chỉnh dựa trên nhiệt độ và độ ẩm
float MQ135_getCorrectionFactor(uint8_t t, uint8_t h);

// Hàm lấy điện trở của cảm biến
float MQ135_getResistance(void);

// Hàm lấy điện trở đã hiệu chỉnh dựa trên nhiệt độ và độ ẩm
float MQ135_getCorrectedResistance(uint8_t t, uint8_t h);

// Hàm lấy giá trị PPM
float MQ135_getPPM(void);

// Hàm lấy giá trị PPM đã hiệu chỉnh dựa trên nhiệt độ và độ ẩm
float MQ135_getCorrectedPPM(uint8_t t, uint8_t h);
// Hàm lấy giá trị RZero
float MQ135_getRZero(void);

// Hàm lấy giá trị RZero đã hiệu chỉnh dựa trên nhiệt độ và độ ẩm
float MQ135_getCorrectedRZero(uint8_t t, uint8_t h);

#endif