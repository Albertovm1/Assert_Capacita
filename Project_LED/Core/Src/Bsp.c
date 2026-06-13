/*******************************************************************************************************/
/**
 *   @file         Bsp.c
 *   @addtogroup   Bsp
 *   @brief        Camada de abstracao de hardware para o projeto.
 *   @author       Alberto Viturino
 *   @details	   Utilizando o Nucleo F767ZI
 *   \n <b>Ferramentas:</b>
 *   - STM32 HAL.
 *
 *   \n <b>Observacoes:</b>
 *   - Toda interface com o hardware deve ser feita exclusivamente por este arquivo.
 *
 *   Changelog
 *   @version      <b>1.0.0 - 23/05/2026</b> \n Alberto Viturino \n Primeira versao
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

#define NUM_LEDS 3

/***********************************************************************************************************************
 * TIPOS LOCAIS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * ESTRUTURAS DE DADOS LOCAIS
 **********************************************************************************************************************/

static GPIO_TypeDef *buttonPort = GPIOA;
static uint16_t buttonPin = GPIO_PIN_3;

/* Portas dos LEDs */
static GPIO_TypeDef *ledPort[NUM_LEDS] =
{
    GPIOA,
    GPIOA,
    GPIOA
};

/* Pinos dos LEDs */
static uint16_t ledPin[NUM_LEDS] =
{
    GPIO_PIN_0,
    GPIO_PIN_1,
    GPIO_PIN_2
};

/***********************************************************************************************************************
 * PROTOTIPOS LOCAIS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * FUNCOES PUBLICAS
 **********************************************************************************************************************/

/********************************************************************************************************
 * @brief   Inicializa os perifericos e as bibliotecas da aplicacao.
 * @details Configura os GPIOs utilizados pelos LEDs e botão,
 *          habilita os clocks necessários e prepara os
 *          periféricos para uso durante a execução do sistema.
 *
 * @warning Esta função deve ser chamada no início do programa,
 *          antes de qualquer função relacionada aos LEDs
 *          ou botão.
 ********************************************************************************************************/

void BSP_Init(void){
    GPIO_InitTypeDef GPIO_InitStruct={0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /* LEDs */

    GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull=GPIO_NOPULL;

    GPIO_InitStruct.Pin=GPIO_PIN_0;
    HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);

    GPIO_InitStruct.Pin=GPIO_PIN_1;
    HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);

    GPIO_InitStruct.Pin=GPIO_PIN_2;
    HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);

    /* Botão */

    GPIO_InitStruct.Pin=GPIO_PIN_3;
    GPIO_InitStruct.Mode=GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull=GPIO_PULLUP;

    HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);
}

/********************************************************************************************************
 * @brief   Define o estado lógico de um LED.
 *
 * @param   led: índice do LED desejado.
 * @param   state: estado lógico desejado para o LED.
 *
 * @details Realiza escrita no GPIO correspondente ao LED informado.
 ********************************************************************************************************/

void BSP_LED_Write(uint8_t led, GPIO_PinState state){
    if(led >= NUM_LEDS)
        return;

    HAL_GPIO_WritePin(
        ledPort[led],
        ledPin[led],
        state
    );
}

/********************************************************************************************************
 * @brief   Alterna o estado lógico de um LED.
 *
 * @param   led: índice do LED desejado.
 *
 * @details Inverte o estado atual do LED informado.
 ********************************************************************************************************/

void BSP_LED_Toggle(uint8_t led){
    if(led >= NUM_LEDS)
        return;

    HAL_GPIO_TogglePin(
        ledPort[led],
        ledPin[led]
    );
}

/********************************************************************************************************
 * @brief   Realiza leitura do estado do botão.
 *
 * @retval  1: botão pressionado.
 * @retval  0: botão não pressionado.
 *
 * @details Realiza leitura do GPIO associado ao botão.
 *          Como o botão utiliza pull-up interno, a leitura
 *          é invertida para tornar o retorno mais intuitivo.
 ********************************************************************************************************/

uint8_t BSP_Button_Read(void){
    return !HAL_GPIO_ReadPin(buttonPort, buttonPin);
}

/***********************************************************************************************************************
 * FUNCOES LOCAIS
 **********************************************************************************************************************/

/** @} */