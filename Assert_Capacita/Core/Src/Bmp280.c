/*******************************************************************************************************/
/**
 *   @file         Bmp280.c
 *   @addtogroup   Bmp280
 *   @brief        Lib para utilizar o sensor BMP280.
 *   @author       Luiz Neto
 *   @details	   A lib atualmente é bloqueante
 *   \n <b>Ferramentas:</b>
 *   - Generic.
 *
 *   \n <b>Dependencias:</b>
 *   - AssertTypes.
 *
 *
 *   \n <b>Observacoes:</b>
 *   - None.
 *
 *   Changelog
 *   @version      <b>1.0.0 - 12/05/2026</b> \n Luiz Neto \n Primeira versao
 *
 *   @copyright
 *   @{
 ********************************************************************************************************/

 /********************************************************************************************************
 *   INCLUDES
 ********************************************************************************************************/

#include "Bmp280.h"
#include "stdio.h"

/***********************************************************************************************************************
 * DEFINES LOCAIS
 **********************************************************************************************************************/


/***********************************************************************************************************************
 * TIPOS LOCAIS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * @brief Estrutura de manipulacao geral da biblioteca
 *
 **********************************************************************************************************************/

static struct bmp280
{
	/// @brief struct com os buffers utilizados para o sensor
	struct buffers
	{
		u8 rxBuffer[32];
		u8 txBuffer[32];
	} buffers;

	/// @brief struct que guarda os valores de calibracao do bmp280
	struct calib
	{
		u16 dig_T1;
		s16 dig_T2;
		s16 dig_T3;
		u16 dig_P1;
		s16 dig_P2;
		s16 dig_P3;
		s16 dig_P4;
		s16 dig_P5;
		s16 dig_P6;
		s16 dig_P7;
		s16 dig_P8;
		s16 dig_P9;
		s32 t_fine;
	} calib;

	/// @brief struct com os ponteiros de funcoes da lib
	struct funcions
	{
		void (*spiTransmit)(u8 *txData, u8 *rxData, u8 size);
		void (*writeChipSelector)(u8 logicLevel);

	}funcions;

	/// @breif variavel que eh utilizada para guardar os ticks da lib
	volatile u32 tickMs;

} bmp280;

/***********************************************************************************************************************
 * PROTOTIPOS LOCAIS
 **********************************************************************************************************************/

static bmp280Return_t BMP280_ReadRegister(u8 addr, u8 *dataOut);
static bmp280Return_t BMP280_WriteRegister(u8 addr, u8 data);
static bmp280Return_t BMP280_ReadMultiplesRegister(u8 addr, u8 qtdBytes, u8 *dataOut);
static bmp280Return_t BMP280_LoadCalibration(void);
static s32 BMP280_CompensateTemp(s32 adcT);
static u32 BMP280_CompensatePress(s32 adcP);
static void BMP280_DelayMs(u32 ms);


/***********************************************************************************************************************
 *   FUNCOES PUBLICAS
 **********************************************************************************************************************/

/********************************************************************************************************
 * @brief   Inicializa o sensor BMP280 atraves da interface SPI.
 * @param   spiTransmitFunc:        ponteiro para funcao de transmissao SPI fornecida pela aplicacao;
 * @param   writeChipSelectorFunc:  ponteiro para funcao de controle do chip select fornecida pela aplicacao.
 * @retval  bmp280Return_t: eBMP280_RETURN_OK em caso de sucesso na inicializacao,
 *                          eBMP280_RETURN_INVALID_ARGUMENT caso algum ponteiro recebido seja nulo,
 *                          eBMP280_RETURN_ERROR caso falhe em qualquer etapa da inicializacao.
 * @details Realiza a sequencia completa de inicializacao do sensor: verifica o
 *          chip ID, executa soft reset, carrega os parametros de calibracao da
 *          NVM, configura o filtro IIR e ativa o modo de medicao normal com
 *          oversampling x16 para pressao e temperatura.
 * @warning Esta funcao deve ser chamada apos a inicializacao do periferico SPI
 *          e dos GPIOs no BSP.
 ********************************************************************************************************/
bmp280Return_t BMP280_Init(void (*spiTransmitFunc)(u8 *txData, u8 *rxData, u8 size),
							void (*writeChipSelectorFunc)(u8 logicLevel))
{
	u8 chipId = 0;

	if ((spiTransmitFunc == dNULL) || (writeChipSelectorFunc == dNULL))
	{
		return eBMP280_RETURN_INVALID_ARGUMENT;
	}

	bmp280.funcions.spiTransmit = spiTransmitFunc;
	bmp280.funcions.writeChipSelector = writeChipSelectorFunc;

	if (BMP280_ReadRegister(dBMP280_ID, &chipId) != eBMP280_RETURN_OK)
	{
		return eBMP280_RETURN_ERROR;
	}

	if (chipId != dBMP280_EXPECTED)
	{
		return eBMP280_RETURN_ERROR;
	}

	if (BMP280_WriteRegister(dBMP280_RESET, dBMP280_RESET_CMD) != eBMP280_RETURN_OK)
	{
		return eBMP280_RETURN_ERROR;
	}

	BMP280_DelayMs(5);

	if (BMP280_LoadCalibration() != eBMP280_RETURN_OK)
	{
		return eBMP280_RETURN_ERROR;
	}

	if (BMP280_WriteRegister(dBMP280_CONFIG, dBMP280_CONFIG_VALUE) != eBMP280_RETURN_OK)
	{
		return eBMP280_RETURN_ERROR;
	}

	if (BMP280_WriteRegister(dBMP280_CTRL_MEAS, dBMP280_CTRL_MEAS_VALUE) != eBMP280_RETURN_OK)
	{
		return eBMP280_RETURN_ERROR;
	}

	return eBMP280_RETURN_OK;
}

/********************************************************************************************************
 * @brief   Rotina de processo do BMP280, deve ser chamada continuamente no loop principal.
 * @details Realiza a leitura de temperatura e pressao e exibe os valores via
 *          printf a cada 1000 ms. Deve ser chamada dentro do while(1) do main
 *          em conjunto com os handlers das demais bibliotecas da aplicacao.
 ********************************************************************************************************/
void BMP280_Handler(void)
{
	float temperature = 0.0f;
	float pressure = 0.0f;

	if (BMP280_TempAndPressReturn(&temperature, &pressure) == eBMP280_RETURN_OK)
	{
	   printf("Temp: %.2f C | Press: %.2f hPa\n", temperature, pressure);
	}
	else
	{
	   printf("Erro na leitura do BMP280\n");
	}

	BMP280_DelayMs(1000);
}

/********************************************************************************************************
 * @brief   Le e retorna a temperatura compensada em graus Celsius.
 * @param   tempOut: ponteiro para variavel onde sera escrita a temperatura compensada em graus Celsius.
 * @retval  bmp280Return_t: eBMP280_RETURN_OK em caso de sucesso na leitura,
 *                          eBMP280_RETURN_INVALID_ARGUMENT caso o ponteiro recebido seja nulo,
 *                          eBMP280_RETURN_ERROR caso a leitura via SPI falhe.
 * @details Faz um burst read dos registradores de dados, extrai o valor cru de
 *          20 bits da temperatura e aplica a formula de compensacao usando os
 *          parametros de calibracao previamente carregados. A funcao tambem
 *          atualiza a variavel t_fine, utilizada pela compensacao de pressao.
 ********************************************************************************************************/
bmp280Return_t BMP280_TempReturn(float *tempOut)
{
	u8 rawData[6] = {0};
	s32 adcT;
	s32 tempCompensated;

	if (tempOut == dNULL)
	{
		return eBMP280_RETURN_INVALID_ARGUMENT;
	}

	if (BMP280_ReadMultiplesRegister(dBMP280_PRESS_MSB, dBMP280_DATA_SIZE, rawData) != eBMP280_RETURN_OK)
	{
		return eBMP280_RETURN_ERROR;
	}

	adcT = ((s32)rawData[3] << 12) | ((s32)rawData[4] << 4) | ((s32)rawData[5] >> 4);
	tempCompensated = BMP280_CompensateTemp(adcT);

	*tempOut = (float)tempCompensated / 100.0f;

	return eBMP280_RETURN_OK;
}

/********************************************************************************************************
 * @brief   Le e retorna a pressao compensada em hPa.
 * @param   pressOut: ponteiro para variavel onde sera escrita a pressao compensada em hPa.
 * @retval  bmp280Return_t: eBMP280_RETURN_OK em caso de sucesso na leitura,
 *                          eBMP280_RETURN_INVALID_ARGUMENT caso o ponteiro recebido seja nulo,
 *                          eBMP280_RETURN_ERROR caso a leitura via SPI falhe.
 * @details Faz um burst read dos registradores de dados, extrai os valores crus
 *          de 20 bits de temperatura e pressao, executa a compensacao da
 *          temperatura para atualizar t_fine e em seguida aplica a formula de
 *          compensacao da pressao usando aritmetica de 64 bits.
 * @warning A compensacao da pressao depende do valor de t_fine atualizado, por
 *          isso a compensacao da temperatura e sempre executada antes mesmo
 *          que o valor nao seja retornado.
 ********************************************************************************************************/
bmp280Return_t BMP280_PressReturn(float *pressOut)
{
	u8 rawData[6] = {0};
	s32 adcT;
	s32 adcP;
	u32 pressCompensated;

	if (pressOut == dNULL)
	{
		return eBMP280_RETURN_INVALID_ARGUMENT;
	}

	if (BMP280_ReadMultiplesRegister(dBMP280_PRESS_MSB, dBMP280_DATA_SIZE, rawData) != eBMP280_RETURN_OK)
	{
		return eBMP280_RETURN_ERROR;
	}

	adcP = ((s32)rawData[0] << 12) | ((s32)rawData[1] << 4) | ((s32)rawData[2] >> 4);
	adcT = ((s32)rawData[3] << 12) | ((s32)rawData[4] << 4) | ((s32)rawData[5] >> 4);

	(void)BMP280_CompensateTemp(adcT);
	pressCompensated = BMP280_CompensatePress(adcP);

	*pressOut = (float)pressCompensated / 25600.0f;

	return eBMP280_RETURN_OK;
}

/********************************************************************************************************
 * @brief   Le e retorna temperatura e pressao compensadas em uma unica chamada.
 * @param   tempOut:  ponteiro para variavel onde sera escrita a temperatura compensada em graus Celsius;
 * @param   pressOut: ponteiro para variavel onde sera escrita a pressao compensada em hPa.
 * @retval  bmp280Return_t: eBMP280_RETURN_OK em caso de sucesso na leitura,
 *                          eBMP280_RETURN_INVALID_ARGUMENT caso algum ponteiro recebido seja nulo,
 *                          eBMP280_RETURN_ERROR caso a leitura via SPI falhe.
 * @details Faz um unico burst read dos registradores de dados para extrair
 *          temperatura e pressao crus, aplicando em seguida ambas as formulas
 *          de compensacao. Esta funcao deve ser preferida quando ambas as
 *          grandezas sao necessarias, pois economiza uma transacao SPI em
 *          relacao a chamar BMP280_TempReturn e BMP280_PressReturn separadamente.
 ********************************************************************************************************/
bmp280Return_t BMP280_TempAndPressReturn(float *tempOut, float *pressOut)
{
	u8 rawData[6] = {0};
	s32 adcT;
	s32 adcP;
	s32 tempCompensated;
	u32 pressCompensated;

	if ((tempOut == dNULL) || (pressOut == dNULL))
	{
		return eBMP280_RETURN_INVALID_ARGUMENT;
	}

	if (BMP280_ReadMultiplesRegister(dBMP280_PRESS_MSB, dBMP280_DATA_SIZE, rawData) != eBMP280_RETURN_OK)
	{
		return eBMP280_RETURN_ERROR;
	}

	adcP = ((s32)rawData[0] << 12) | ((s32)rawData[1] << 4) | ((s32)rawData[2] >> 4);
	adcT = ((s32)rawData[3] << 12) | ((s32)rawData[4] << 4) | ((s32)rawData[5] >> 4);

	tempCompensated = BMP280_CompensateTemp(adcT);
	pressCompensated = BMP280_CompensatePress(adcP);

	*tempOut = (float)tempCompensated / 100.0f;
	*pressOut = (float)pressCompensated / 25600.0f;

	return eBMP280_RETURN_OK;
}

/********************************************************************************************************
 * @brief   Rotina de tick da biblioteca BMP280.
 * @details Deve ser chamada a cada 1 ms a partir de uma interrupcao de timer.
 *          Fornece a base de tempo interna utilizada para temporizar o processo
 *          de inicializacao do sensor.
 * @warning Esta funcao deve ser implementada em um timer de frequencia 1000 Hz.
 ********************************************************************************************************/
void BMP280_Tick(void)
{
    bmp280.tickMs++;
}

/***********************************************************************************************************************
 *   FUNCOES LOCAIS
 **********************************************************************************************************************/

/********************************************************************************************************
 * @brief   Le o valor de um unico registrador do sensor via SPI.
 * @param   addr:    endereco do registrador a ser lido;
 * @param   dataOut: ponteiro para variavel onde sera escrito o byte lido.
 * @retval  bmp280Return_t: eBMP280_RETURN_OK em caso de sucesso,
 *                          eBMP280_RETURN_INVALID_ARGUMENT caso o ponteiro recebido seja nulo.
 * @details Monta um pacote SPI de 2 bytes contendo o endereco com o bit 7
 *          setado (indicando operacao de leitura) e um byte dummy. O primeiro
 *          byte recebido durante a transmissao do endereco e descartado e o
 *          dado util retorna na segunda posicao do buffer de recepcao.
 ********************************************************************************************************/
static bmp280Return_t BMP280_ReadRegister(u8 addr, u8 *dataOut)
{
	if (dataOut == dNULL)
	{
		return eBMP280_RETURN_INVALID_ARGUMENT;
	}

	bmp280.buffers.txBuffer[0] = dBIT_SET(addr, 7);
	bmp280.buffers.txBuffer[1] = 0x00;

	bmp280.buffers.rxBuffer[0] = 0x00;
	bmp280.buffers.rxBuffer[1] = 0x00;

	bmp280.funcions.writeChipSelector(dFALSE);
	bmp280.funcions.spiTransmit(bmp280.buffers.txBuffer, bmp280.buffers.rxBuffer, 2);
	bmp280.funcions.writeChipSelector(dTRUE);

	*dataOut = bmp280.buffers.rxBuffer[1];

	return eBMP280_RETURN_OK;
}

/********************************************************************************************************
 * @brief   Escreve um valor em um registrador do sensor via SPI.
 * @param   addr: endereco do registrador a ser escrito;
 * @param   data: byte de dado a ser escrito no registrador.
 * @retval  bmp280Return_t: eBMP280_RETURN_OK em caso de sucesso na transmissao.
 * @details Monta um pacote SPI de 2 bytes contendo o endereco com o bit 7
 *          resetado (indicando operacao de escrita) seguido do byte de dado.
 ********************************************************************************************************/
static bmp280Return_t BMP280_WriteRegister(u8 addr, u8 data)
{
	bmp280.buffers.txBuffer[0] = dBIT_CLEAR(addr, 7);
	bmp280.buffers.txBuffer[1] = data;

	bmp280.funcions.writeChipSelector(dFALSE);
	bmp280.funcions.spiTransmit(bmp280.buffers.txBuffer, bmp280.buffers.rxBuffer, 2);
	bmp280.funcions.writeChipSelector(dTRUE);

	return eBMP280_RETURN_OK;
}

/********************************************************************************************************
 * @brief   Le multiplos registradores em burst a partir de um endereco inicial.
 * @param   addr:     endereco do primeiro registrador a ser lido;
 * @param   qtdBytes: quantidade de bytes a serem lidos em sequencia;
 * @param   dataOut:  ponteiro para buffer onde serao escritos os bytes lidos.
 * @retval  bmp280Return_t: eBMP280_RETURN_OK em caso de sucesso,
 *                          eBMP280_RETURN_INVALID_ARGUMENT caso o ponteiro recebido seja nulo
 *                          ou caso a quantidade solicitada exceda o tamanho do buffer interno.
 * @details Aproveita o auto-incremento de endereco do BMP280 para ler varios
 *          registradores em uma unica transacao SPI. O primeiro byte da
 *          resposta e descartado por corresponder ao envio do endereco e os
 *          dados uteis comecam no segundo byte do buffer de recepcao.
 * @warning O tamanho do buffer interno limita a quantidade maxima de bytes
 *          que podem ser lidos em uma unica chamada.
 ********************************************************************************************************/
static bmp280Return_t BMP280_ReadMultiplesRegister(u8 addr, u8 qtdBytes, u8 *dataOut)
{
	u8 i;

	if (dataOut == dNULL)
	{
		return eBMP280_RETURN_INVALID_ARGUMENT;
	}

	if ((qtdBytes + 1) > sizeof(bmp280.buffers.txBuffer))
	{
		return eBMP280_RETURN_INVALID_ARGUMENT;
	}

	bmp280.buffers.txBuffer[0] = dBIT_SET(addr, 7);
	for (i = 1; i <= qtdBytes; i++)
	{
		bmp280.buffers.txBuffer[i] = 0x00;
		bmp280.buffers.rxBuffer[i] = 0x00;
	}

	bmp280.funcions.writeChipSelector(dFALSE);
	bmp280.funcions.spiTransmit(bmp280.buffers.txBuffer, bmp280.buffers.rxBuffer, qtdBytes + 1);
	bmp280.funcions.writeChipSelector(dTRUE);

	for (i = 0; i < qtdBytes; i++)
	{
		dataOut[i] = bmp280.buffers.rxBuffer[i + 1];
	}

	return eBMP280_RETURN_OK;
}

/********************************************************************************************************
 * @brief   Le os parametros de calibracao da NVM e armazena na struct interna.
 * @retval  bmp280Return_t: eBMP280_RETURN_OK em caso de sucesso na leitura,
 *                          eBMP280_RETURN_ERROR caso a leitura via SPI falhe.
 * @details Le os 24 bytes de calibracao a partir do endereco 0x88 e os
 *          interpreta como 12 palavras de 16 bits em complemento de dois,
 *          combinando os bytes LSB e MSB de cada par. Os valores sao usados
 *          posteriormente nas formulas de compensacao de temperatura e pressao.
 ********************************************************************************************************/
static bmp280Return_t BMP280_LoadCalibration(void)
{
	u8 calibData[dBMP280_CALIB_SIZE] = {0};

	if (BMP280_ReadMultiplesRegister(dBMP280_CALIB_START, dBMP280_CALIB_SIZE, calibData) != eBMP280_RETURN_OK)
	{
		return eBMP280_RETURN_ERROR;
	}

	bmp280.calib.dig_T1 = (u16)((calibData[1]  << 8) | calibData[0]);
	bmp280.calib.dig_T2 = (s16)((calibData[3]  << 8) | calibData[2]);
	bmp280.calib.dig_T3 = (s16)((calibData[5]  << 8) | calibData[4]);

	bmp280.calib.dig_P1 = (u16)((calibData[7]  << 8) | calibData[6]);
	bmp280.calib.dig_P2 = (s16)((calibData[9]  << 8) | calibData[8]);
	bmp280.calib.dig_P3 = (s16)((calibData[11] << 8) | calibData[10]);
	bmp280.calib.dig_P4 = (s16)((calibData[13] << 8) | calibData[12]);
	bmp280.calib.dig_P5 = (s16)((calibData[15] << 8) | calibData[14]);
	bmp280.calib.dig_P6 = (s16)((calibData[17] << 8) | calibData[16]);
	bmp280.calib.dig_P7 = (s16)((calibData[19] << 8) | calibData[18]);
	bmp280.calib.dig_P8 = (s16)((calibData[21] << 8) | calibData[20]);
	bmp280.calib.dig_P9 = (s16)((calibData[23] << 8) | calibData[22]);

	return eBMP280_RETURN_OK;
}

/********************************************************************************************************
 * @brief   Compensa o valor cru de temperatura usando os parametros de calibracao.
 * @param   adcT: valor cru de temperatura de 20 bits lido do sensor.
 * @retval  s32: temperatura compensada em centesimos de grau Celsius
 *               (valor 5123 equivale a 51.23 graus C).
 * @details Aplica a formula de compensacao de temperatura recomendada pela
 *          Bosch usando aritmetica inteira de 32 bits. A funcao tambem
 *          atualiza a variavel t_fine, que carrega o valor de temperatura em
 *          resolucao fina utilizado pela compensacao da pressao.
 * @warning Esta funcao deve ser chamada antes de BMP280_CompensatePress, pois
 *          a compensacao da pressao depende do valor atualizado de t_fine.
 ********************************************************************************************************/
static s32 BMP280_CompensateTemp(s32 adcT)
{
	s32 var1;
	s32 var2;
	s32 T;

	var1 = ((((adcT >> 3) - ((s32)bmp280.calib.dig_T1 << 1))) * ((s32)bmp280.calib.dig_T2)) >> 11;
	var2 = (((((adcT >> 4) - ((s32)bmp280.calib.dig_T1)) * ((adcT >> 4) - ((s32)bmp280.calib.dig_T1))) >> 12) *
			((s32)bmp280.calib.dig_T3)) >> 14;

	bmp280.calib.t_fine = var1 + var2;
	T = (bmp280.calib.t_fine * 5 + 128) >> 8;

	return T;
}

/********************************************************************************************************
 * @brief   Compensa o valor cru de pressao usando os parametros de calibracao.
 * @param   adcP: valor cru de pressao de 20 bits lido do sensor.
 * @retval  u32: pressao compensada em formato Q24.8 Pa (24 bits de parte inteira
 *               e 8 bits de parte fracionaria). O valor 24674867 equivale a
 *               24674867/256 = 96386.2 Pa = 963.862 hPa.
 * @details Aplica a formula de compensacao de pressao recomendada pela Bosch
 *          usando aritmetica inteira de 64 bits para garantir a precisao
 *          maxima. O calculo depende da variavel global t_fine, que deve ter
 *          sido atualizada por uma chamada previa a BMP280_CompensateTemp.
 * @warning Retorna zero caso o valor intermediario var1 seja zero, evitando
 *          divisao por zero. Esta condicao indica calibracao invalida.
 ********************************************************************************************************/
static u32 BMP280_CompensatePress(s32 adcP)
{
	s64 var1;
	s64 var2;
	s64 p;

	var1 = ((s64)bmp280.calib.t_fine) - 128000;
	var2 = var1 * var1 * (s64)bmp280.calib.dig_P6;
	var2 = var2 + ((var1 * (s64)bmp280.calib.dig_P5) << 17);
	var2 = var2 + (((s64)bmp280.calib.dig_P4) << 35);
	var1 = ((var1 * var1 * (s64)bmp280.calib.dig_P3) >> 8) + ((var1 * (s64)bmp280.calib.dig_P2) << 12);
	var1 = (((((s64)1) << 47) + var1)) * ((s64)bmp280.calib.dig_P1) >> 33;

	if (var1 == 0)
	{
		return 0;
	}

	p = 1048576 - adcP;
	p = (((p << 31) - var2) * 3125) / var1;
	var1 = (((s64)bmp280.calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
	var2 = (((s64)bmp280.calib.dig_P8) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + (((s64)bmp280.calib.dig_P7) << 4);

	return (u32)p;
}

/********************************************************************************************************
 * @brief   Delay bloqueante baseado no tick interno da biblioteca.
 * @param   ms: tempo de espera em milissegundos.
 * @details Utiliza o contador tickMs atualizado por BMP280_Tick para realizar
 *          uma espera bloqueante sem depender de funcoes externas de delay.
 * @warning Requer que BMP280_Tick seja chamada a cada 1 ms via interrupcao de
 *          timer para que o tempo seja preciso.
 ********************************************************************************************************/
static void BMP280_DelayMs(u32 ms)
{
    u32 tickStart = bmp280.tickMs;

    while ((bmp280.tickMs - tickStart) < ms)
    {
        // aguarda o tempo solicitado
    }
}
