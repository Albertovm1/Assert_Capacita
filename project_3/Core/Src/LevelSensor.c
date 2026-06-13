/*******************************************************************************************************/
/**
 *   @file         LevelSensor.c
 *   @addtogroup   LevelSensor
 *   @brief        Logica de negocio e filtragem do sensor de nivel do tanque.
 *   @author       Alberto Viturino
 *   @details      Acumula amostras do ADC e calcula a media ponderada para converter
 *                 o nivel em percentual (0 a 100%).
 *
 *   Changelog
 *   @version      <b>1.0.0 - 13/06/2026</b> \n Alberto Viturino \n Primeira versao
 *
 *   @copyright
 *   @{
 ********************************************************************************************************/

/***********************************************************************************************************************
 * INCLUDES
 **********************************************************************************************************************/

#include "LevelSensor.h"

/***********************************************************************************************************************
 * DEFINES LOCAIS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * TIPOS LOCAIS
 **********************************************************************************************************************/

/**
 * @brief Estrutura de controle interna do sensor de nivel.
 */
typedef struct {
    uint32_t acumulador;
    uint16_t contador;  
    uint8_t  percentagem;
} tsLevelSensor;

/***********************************************************************************************************************
 * ESTRUTURAS DE DADOS LOCAIS
 **********************************************************************************************************************/

/* Instancia local e protegida da estrutura do sensor */
static tsLevelSensor levelSensor;

/***********************************************************************************************************************
 * PROTOTIPOS LOCAIS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * FUNCOES PUBLICAS
 **********************************************************************************************************************/

/********************************************************************************************************
 * @brief   Inicializa a estrutura de dados do sensor de nivel.
 * @details Zera o acumulador, contador de amostras e o percentual armazenado.
 ********************************************************************************************************/
void LevelSensor_Init(void)
{
    levelSensor.acumulador    = 0;
    levelSensor.contador    = 0;
    levelSensor.percentagem = 0;
}

/********************************************************************************************************
 * @brief   Acumula uma nova amostra e processa a media ao atingir o limite.
 * @param   rawValue Leitura digital bruta vinda do ADC (0 a 4095).
 * @details Quando o numero de amostras atinge dLEVEL_SENSOR_NUMBER, a media eh calculada,
 *          convertida para percentual de 0 a 100% e os contadores sao reiniciados.
 ********************************************************************************************************/
void LevelSensor_NewSample(uint16_t rawValue)
{
    levelSensor.acumulador += rawValue;
    levelSensor.contador++;

    //Verifica se ja coletamos o numero necessario de amostras, que é 20;
    if (levelSensor.contador >= dLEVEL_SENSOR_NUMBER)
    {
        // calcula a media 
        uint32_t averageRaw = levelSensor.acumulador / dLEVEL_SENSOR_NUMBER;

        // converte em porcentagem
        //Formula: (Media * 100) / Max_ADC 
        levelSensor.percentagem = (uint8_t)((averageRaw * 100U) / dLEVEL_SENSOR_ADC_MAX);

        // reset para o proximo bloco de codigo
        levelSensor.acumulador = 0;
        levelSensor.contador = 0;
    }
}

/********************************************************************************************************
 * @brief   Obtem o ultimo percentual calculado do tanque.
 * @return  uint8_t Valor de 0 a 100%.
 ********************************************************************************************************/
 
uint8_t LevelSensor_GetPercent(void)
{
    return levelSensor.percentagem;
}

/***********************************************************************************************************************
 * FUNCOES LOCAIS
 **********************************************************************************************************************/

/** @} */