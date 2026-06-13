/*******************************************************************************************************/
/**
 *   @file         Bsp.c
 *   @addtogroup   Bsp
 *   @brief        Camada de abstracao de hardware para o projeto.
 *   @author       Luiz Neto
 *   @details	   Utilizando o Nucleo F767ZI
 *   \n <b>Ferramentas:</b>
 *   - STM32 HAL.
 *
 *   \n <b>Dependencias:</b>
 *   - AssertTypes.
 *   - Bmp280.
 *
 *   \n <b>Observacoes:</b>
 *   - Toda interface com o hardware deve ser feita exclusivamente por este arquivo.
 *
 *   Changelog
 *   @version      <b>1.0.0 - 15/05/2026</b> \n Luiz Neto \n Primeira versao
 *
 *   @copyright
 *   @{
 ********************************************************************************************************/

/***********************************************************************************************************************
 * INCLUDES
 **********************************************************************************************************************/

#include "Bsp.h"
#include "main.h"

/***********************************************************************************************************************
 * DEFINES LOCAIS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * TIPOS LOCAIS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * ESTRUTURAS DE DADOS LOCAIS
 **********************************************************************************************************************/

extern SPI_HandleTypeDef hspi1;
extern TIM_HandleTypeDef htim6;

/***********************************************************************************************************************
 * PROTOTIPOS LOCAIS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *   FUNCOES PUBLICAS
 **********************************************************************************************************************/

/********************************************************************************************************
 * @brief   Inicializa os perifericos e as bibliotecas da aplicacao.
 * @details Deve ser chamada no inicio do main antes de qualquer outra funcao
 *          da aplicacao. Inicializa o timer de base de tempo e o sensor BMP280.
 * @warning O timer TIM6 deve estar configurado para gerar interrupcao a cada
 *          1 ms antes de chamar esta funcao.
 ********************************************************************************************************/
void Bsp_Init(void)
{
    HAL_TIM_Base_Start_IT(&htim6);

    HAL_Delay(2000);

	if (BMP280_Init(Bsp_Spi, Bsp_GpioWrite) == eBMP280_RETURN_OK)
	{
	   printf("BMP Inicializado com sucesso\n");
	}
	else
	{
	   printf("BMP nao foi incializado\n");
	   while(1);
	}
}

/********************************************************************************************************
 * @brief   Rotina de tick da camada BSP chamada a cada 1 ms pela ISR do TIM6.
 * @details Repassa o tick para todas as bibliotecas que necessitam de base de
 *          tempo. Deve ser chamada dentro do HAL_TIM_PeriodElapsedCallback.
 * @warning Esta funcao deve ser chamada exclusivamente pela ISR do TIM6.
 ********************************************************************************************************/
void Bsp_Tick(void)
{
    BMP280_Tick();
}

/********************************************************************************************************
 * @brief   Realiza uma transmissao e recepcao SPI via SPI1.
 * @param   txData: ponteiro para o buffer de dados a serem transmitidos;
 * @param   rxData: ponteiro para o buffer onde serao armazenados os dados recebidos;
 * @param   size:   quantidade de bytes a serem transmitidos e recebidos.
 * @details Encapsula a chamada HAL_SPI_TransmitReceive para o periferico SPI1
 *          em modo bloqueante. Utilizada como callback de SPI pela biblioteca
 *          do BMP280.
 ********************************************************************************************************/
void Bsp_Spi(u8 *txData, u8 *rxData, u8 size)
{
    HAL_SPI_TransmitReceive(&hspi1, txData, rxData, size, HAL_MAX_DELAY);
}

/********************************************************************************************************
 * @brief   Controla o nivel logico do pino de chip select do BMP280 (PC7).
 * @param   levelLogic: nivel logico desejado para o pino (dTRUE para alto,
 *                      dFALSE para baixo).
 * @details Encapsula a chamada HAL_GPIO_WritePin para o pino PC7, utilizado
 *          como chip select do sensor BMP280. Utilizada como callback de GPIO
 *          pela biblioteca do BMP280.
 ********************************************************************************************************/
void Bsp_GpioWrite(u8 levelLogic)
{
    if (levelLogic == dTRUE)
    {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
    }
}

/***********************************************************************************************************************
 *   FUNCOES LOCAIS
 **********************************************************************************************************************/

/** @} */
