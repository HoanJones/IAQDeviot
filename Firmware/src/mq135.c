#include <simcom_api.h>
#include "mq135.h"
#include "simcom_pm.h"
#include <math.h>
//#include "..\inc\adc.h"
#include "..\inc\debug.h"

#define ADC_val sAPI_ReadVbat()
#define MQ_TASK 1

static sTaskRef mqTask = NULL;
static UINT8 mqTaskStack[1024*3];

float CO2_val = 0.0;

/********************Function process data From MQ135************************************/
//static void MQ135_processTask(void* ptr);
/*!
@brief  Get the correction factor to correct for temperature and humidity

@param[in] t  The ambient air temperature
@param[in] h  The relative humidity

@return The calculated correction factor
*/
/**************************************************************************/
float MQ135_getCorrectionFactor(uint8_t t, uint8_t h) {
  return CORA * t * t - CORB * t + CORC - (h-33.)*CORD;
}
float MQ135_getResistance() {
    return ((4* (1023./(float)ADC_val)) * 5. - 1.)*RLOAD;
}

/*!
@brief  Get the resistance of the sensor, ie. the measurement value corrected
        for temp/hum

@param[in] t  The ambient air temperature
@param[in] h  The relative humidity

@return The corrected sensor resistance kOhm
*/
/**************************************************************************/
float MQ135_getCorrectedResistance(uint8_t t, uint8_t h) {
    return MQ135_getResistance() / MQ135_getCorrectionFactor(t, h);
}

/**************************************************************************/
/*!
@brief  Get the ppm of CO2 sensed (assuming only CO2 in the air)

@return The ppm of CO2 in the air
*/
/**************************************************************************/
float MQ135_getPPM() {
    return (ATMOCO2 + (PARA * pow((MQ135_getResistance()/RZERO), -PARB)));
}
/**************************************************************************/
/*!
@brief  Get the ppm of CO2 sensed (assuming only CO2 in the air), corrected
        for temp/hum

@param[in] t  The ambient air temperature
@param[in] h  The relative humidity

@return The ppm of CO2 in the air
*/
/**************************************************************************/
float MQ135_getCorrectedPPM(uint8_t t, uint8_t h) {
    return ATMOCO2 + (PARA * pow((MQ135_getCorrectedResistance(t, h)/RZERO), -PARB));
}

/**************************************************************************/
/*!
@brief  Get the resistance RZero of the sensor for calibration purposes

@return The sensor resistance RZero in kOhm
*/
/**************************************************************************/
float MQ135_getRZero() {   
    return MQ135_getResistance() * pow((ATMOCO2/PARA), (1./PARB));
}

/**************************************************************************/
/*!
@brief  Get the corrected resistance RZero of the sensor for calibration
        purposes

@param[in] t  The ambient air temperature
@param[in] h  The relative humidity

@return The corrected sensor resistance RZero in kOhm
*/
/**************************************************************************/
float MQ135_getCorrectedRZero(uint8_t t, uint8_t h) {
    return MQ135_getCorrectedResistance(t, h) * pow((ATMOCO2/PARA), (1./PARB));
}


void MQ135_init() {
    // if(MQ_TASK) {
    //     if (sAPI_TaskCreate(&mqTask, mqTaskStack, 3*1024, 90, (char*)"MQ135 process", 
    // MQ135_processTask, (void *)0) != SC_SUCCESS ) {
    //     DEBUG(__FILENAME__, "Create mqTask fail! \r\n");
    // }
    // DEBUG(__FILENAME__, "MQ135_Task ok!, start read ADC from MQ135 \r\n");
    // }

    CO2_val =  MQ135_getPPM();
    //DEBUG(__FILENAME__, "Nong do CO2 trong khong khi: [%0.f] PPM \r\n", CO2_val);

}

/**
void MQ135_processTask(void *ptr) {

    //uint8_t CO2_val;

    while(1) {
        CO2_val = MQ135_getPPM();

        DEBUG(__FILENAME__, "Nong do CO2 trong khong khi: [%0.f] PPM\r\n", CO2_val);
        sAPI_TaskSleep(200); //2s
    }
}
/**/