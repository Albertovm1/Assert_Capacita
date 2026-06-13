/*******************************************************************************************************/
/**
 *   @file         Bsp.c
 *   @addtogroup   Bsp
 *   @brief        Camada de abstracao de hardware para o projeto.
 *   @author       Alberto Viturino
 *   @details      Utilizando o Nucleo F767ZI
 *   \n <b>Ferramentas:</b>
 *   - STM32 HAL.
 *
 *   \n <b>Observacoes:</b>
 *   - Toda interface com o hardware deve ser feita exclusivamente por este arquivo.
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

#include "Bsp.h"
#include "main.h"
#include <stdio.h>

/***********************************************************************************************************************
 * DEFINES LOCAIS
 **********************************************************************************************************************/

/* Configurações padrão do canal analógico (Exemplo: Canal associado ao potenciômetro) */
#define porta GPIOA
#define pino  GPIO_PIN_3 

/***********************************************************************************************************************
 * TIPOS LOCAIS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * ESTRUTURAS DE DADOS LOCAIS
 **********************************************************************************************************************/

/* Handles dos periféricos gerenciados pela HAL (declarados originalmente no main.c) */
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim6;
extern UART_HandleTypeDef huart3;

/* Variável de controle interna para o estouro do timer de 50ms */
static volatile uint8_t timer_expired = 0;

/***********************************************************************************************************************
 * PROTOTIPOS LOCAIS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * FUNCOES PUBLICAS
 **********************************************************************************************************************/

/********************************************************************************************************
 * @brief   Inicializa a camada de abstração de hardware (BSP).
 * @details Esta função prepara os periféricos da placa e inicializa o timer base de tempo de 50ms.
 *
 * @warning Deve ser chamada no início do main.c, após a inicialização nativa da HAL.
 *     
 ********************************************************************************************************/
void Bsp_Init(void)
{
    /* Inicializa o Timer 6 em modo de interrupção imediatamente */
    Bsp_Timer_Start();
}

/********************************************************************************************************
 * @brief   Realiza a leitura analógica do nível do tanque por Polling.
 * @details Bloqueia momentaneamente a execução até que a conversão do ADC esteja concluída.
 *
 * @return  uint32_t Valor digitalizado bruto convertido (Resolução de 0 a 4095 para 12 bits).
 ********************************************************************************************************/

uint32_t Bsp_ADC_Read(void)
{
    uint32_t adc_val = 0;

    HAL_ADC_Start(&hadc1); // Liga o ADC

    if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) //Espera a leitura acabar
    {
        adc_val = HAL_ADC_GetValue(&hadc1); //Pega o valor digital (0 a 4095)
    }

    HAL_ADC_Stop(&hadc1); //Desliga o ADC

    return adc_val;
}

/********************************************************************************************************
 * @brief   Inicia a contagem do Timer básico (TIM6).
 * @details Configura o Timer para operar disparando interrupções periódicas a cada 50ms.
 *
 ********************************************************************************************************/
void Bsp_Timer_Start(void)
{
    HAL_TIM_Base_Start_IT(&htim6);
}

/********************************************************************************************************
 * @brief   Verifica se o período de amostragem estourou.
 * @details Executa a checagem da flag interna atualizando-a caso tenha ocorrido o timeout.
 *
 * @return  uint8_t Retorna 1 se o tempo de 50ms expirou, ou 0 caso contrário.
 ********************************************************************************************************/
uint8_t Bsp_Timer_CheckTimeout(void)
{
    uint8_t status = 0;

    if (timer_expired != 0)
    {
        timer_expired = 0; /* Limpa o flag para o próximo ciclo */
        status = 1;        /* Timeout confirmado */
    }

    return status;
}

/***********************************************************************************************************************
 * FUNCOES LOCAIS
 **********************************************************************************************************************/

/********************************************************************************************************
 * @brief   Callback da HAL executado no estouro do período do Timer.
 * @param   htim Ponteiro para a estrutura de controle do Timer que gerou a interrupção.
 * @details Identifica se a interrupção pertence ao TIM6 e sinaliza o estouro do ciclo de 50ms.
 ********************************************************************************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6)
    {
        timer_expired = 1; /* Sinaliza o estouro do tempo */
    }
}

/********************************************************************************************************
 * @brief   Redireciona a saída padrão de caracteres (printf) para o periférico USART3.
 * @param   ch Caractere ASCII enviado pelo buffer do printf.
 * @details Transmite o caractere byte a byte de forma síncrona via UART.
 * @return  int Retorna o próprio caractere transmitido.
 ********************************************************************************************************/

 int __io_putchar(int ch){
    HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 10);
    return ch;
}

/** @} */