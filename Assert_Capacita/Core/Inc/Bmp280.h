/***********************************************************************************************************************
 *   @file Bmp280.h
 *   @addtogroup Bmp280
 *   @{
 **********************************************************************************************************************/

#ifndef INC_BMP280_H_
#define INC_BMP280_H_

 /********************************************************************************************************
 *   INCLUDES
 ********************************************************************************************************/

#include "AssertTypes.h"

/***********************************************************************************************************************
 * DEFINES PUBLICOS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  @addtogroup BMP280_Register
 *  @brief      Define os valores dos registradores do sensor.
 *  @{
**********************************************************************************************************************/

/// Valor do registrador que retorna o ID
#define dBMP280_ID (0xD0U)

/// Valor do registrador de reset
#define dBMP280_RESET (0xE0U)

/// Valor do registrador de status
#define dBMP280_STATUS (0xF3U)

/// Valor do registrador de Controle
#define dBMP280_CTRL_MEAS (0xF4U)

/// Valor do registrador de Configuracao
#define dBMP280_CONFIG (0xF5U)

/// Valor do registrador de dados da pressao (MSB)
#define dBMP280_PRESS_MSB (0xF7U)

/// Valor do registrador de dados da pressao (LSB)
#define dBMP280_PRESS_LSB (0xF8U)

/// Valor do registrador de dados da pressao (XLSB)
#define dBMP280_PRESS_XLSB (0xF9U)

/// Valor do registrador de dados da temperatura (MSB)
#define dBMP280_TEMP_MSB (0xFAU)

/// Valor do registrador de dados da temperatura (LSB)
#define dBMP280_TEMP_LSB (0xFBU)

/// Valor do registrador de dados da temperatura (XLSB)
#define dBMP280_TEMP_XLSB (0xFCU)

/// Endereco inicial dos registradores de calibracao
#define dBMP280_CALIB_START (0x88U)

/** @} BMP280_Register */

/***********************************************************************************************************************
 *  @addtogroup BMP280_Config
 *  @brief      Defines das configs padroes da lib.
 *  @{
**********************************************************************************************************************/

/// Define o valor padrao do ID
#define dBMP280_EXPECTED (0x58U)

/// Comando de soft reset
#define dBMP280_RESET_CMD (0xB6U)

/// Quantidade de bytes de calibracao
#define dBMP280_CALIB_SIZE (24U)

/// Quantidade de bytes de dados (press + temp)
#define dBMP280_DATA_SIZE (6U)

/// Valor padrao do registrador config (filtro IIR coef = 16, t_sb = 0.5ms)
#define dBMP280_CONFIG_VALUE (0x1CU)

/// Valor padrao do registrador ctrl_meas (osrs_t x16, osrs_p x16, normal mode)
#define dBMP280_CTRL_MEAS_VALUE (0xFFU)

/** @} BMP280_Config */

/********************************************************************************************************
 *   TIPOS DE DADOS PUBLICOS
 ********************************************************************************************************/

/// @brief Retornos para controle de erros das funcoes do BMP280
typedef enum bmp280Return_t
{
	eBMP280_RETURN_OK,
	eBMP280_RETURN_ERROR,
	eBMP280_RETURN_INVALID_ARGUMENT,

	eBMP280_RETURN_END_ENUM
} bmp280Return_t;

/***********************************************************************************************************************
 * PROTOTIPOS PUBLICOS
 **********************************************************************************************************************/

bmp280Return_t BMP280_Init(void (*spiTransmitFunc)(u8 *txData, u8 *rxData, u8 size),
							void (*writeChipSelectorFunc)(u8 logicLevel));
void BMP280_Handler(void);
bmp280Return_t BMP280_TempReturn(float *tempOut);
bmp280Return_t BMP280_PressReturn(float *pressOut);
bmp280Return_t BMP280_TempAndPressReturn(float *tempOut, float *pressOut);
void BMP280_Tick(void);

#endif /* INC_BMP280_H_ */

