/*******************************************************************************************************/
/**
 *   @file         LevelSensor.h
 *   @addtogroup   LevelSensor
 *   @{
 ********************************************************************************************************/

 
#ifndef LEVEL_SENSOR_H
#define LEVEL_SENSOR_H

/***********************************************************************************************************************
 * INCLUDES NECESSARIOS
 **********************************************************************************************************************/

#include "main.h"

/***********************************************************************************************************************
 * PROTOTIPOS PUBLICOS
 **********************************************************************************************************************/

// incializar

#define dLEVEL_SENSOR_NUMBER   20    //Numero de amostras para o filtro de media
#define dLEVEL_SENSOR_ADC_MAX  4095 //Valor maximo do ADC de 12 bits = 2^12 - 1 = 4095

// incializar o sensor
void    LevelSensor_Init(void);

// nova amostra do sensor
uint8_t LevelSensor_NewSample(uint16_t rawValue);

// valor do nivel em porcentagems
uint8_t LevelSensor_GetPercent(void);

#endif /* LEVEL_SENSOR_H */