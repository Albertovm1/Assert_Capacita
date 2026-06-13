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
uint8_t LevelSensor_NewSample(uint16_t rawValue)
{
    uint8_t media_pronta = 0;

    levelSensor.acumulador += rawValue;
    levelSensor.contador++;

    /* Verifica se atingiu o bloco de 20 amostras */
    if (levelSensor.contador >= dLEVEL_SENSOR_NUMBER)
    {
        uint32_t averageRaw = levelSensor.acumulador / dLEVEL_SENSOR_NUMBER;

        /* Converte para porcentagem */
        levelSensor.percentagem = (uint8_t)((averageRaw * 100U) / dLEVEL_SENSOR_ADC_MAX);

        /* Reseta para o próximo ciclo */
        levelSensor.acumulador = 0;
        levelSensor.contador = 0;

        media_pronta = 1; /* Sinaliza que atualizou o valor real */
    }

    return media_pronta;
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