/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>
#include <string.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

#define RX_BUFFER_SIZE 256
#define TX_BUFFER_SIZE 512
#define JSON_TOKEN_NOT_FOUND  0xFFFF
#define DEVICE_ID_BROADCAST     0xFF
#define SAFETY_GAP                40

#define EN_TX_485() { HAL_GPIO_WritePin(U1_TXEN_485_GPIO_Port, U1_TXEN_485_Pin, 1); }
#define EN_RX_485() { HAL_GPIO_WritePin(U1_TXEN_485_GPIO_Port, U1_TXEN_485_Pin, 0); }


/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

uint8_t txBuffer[TX_BUFFER_SIZE];
uint8_t rxIndex, rxRcvd, rxBuffer[RX_BUFFER_SIZE];

uint16_t anValues[3], angolo4096;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

long int ParserJSON(const char *token) {
	int i, j, ap, ch, done, trovato;

	long int retval = JSON_TOKEN_NOT_FOUND;

	/* il buffer è circolare, potrebbe trovarsi a cavalcioni,
	 * quindi prima cosa cerco la graffa aperta */
	for (i=0, ap=-1, ch=-1; i<RX_BUFFER_SIZE; i++) {
		if (rxBuffer[i] == '{') { ap = i; }
		if (rxBuffer[i] == '}') { ch = i; }
	}

	if ((ap >= 0) && (ch >= 0)) {

		/* ci sono tutte le graffe, cerco il token desiderato */
		for (i=ap+1, j=0, trovato=0, done=0; done==0; i++) {
			if (i >= RX_BUFFER_SIZE) { i = i % RX_BUFFER_SIZE; }

			if ((rxBuffer[i] == ':') && (token[j]==0)) { trovato=1; done=1; }
			else if (rxBuffer[i] == token[j]) { j++; }
			else { j=0; }

			if (rxBuffer[i] == '}') { done=1; }
		}

		if (trovato != 0) {
			/* trovato il token, tiro fuori il valore */
			retval = 0;
			for (done=0; done==0; i++) {
				if (i >= RX_BUFFER_SIZE) { i = i % RX_BUFFER_SIZE; }

				if ((rxBuffer[i] == ',') || (rxBuffer[i] == '}')) { done=1; }
				else if ((rxBuffer[i] >= '0') && (rxBuffer[i] <= '9')) {
					retval = (retval*10) + rxBuffer[i] - '0';
				}
			}
		}
	}

	return retval;
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

	uint32_t status;
	int device_id;
	int32_t direction, steps_to_do;
	int32_t deg4096_to_go;

	deg4096_to_go = -1;

	/* direction : 1=avanti  -1=indietro */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

/* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */


  /* A4988 MS3, MS2, MS1 LOW  LOW  LOW  =   FULL STEP  */
  /* A4988 MS3, MS2, MS1 LOW  LOW  HIGH =   HALF STEP  */
  /* A4988 MS3, MS2, MS1 LOW  HIGH LOW  =  4 microstep */
  /* A4988 MS3, MS2, MS1 LOW  HIGH HIGH =  8 microstep */
  /* A4988 MS3, MS2, MS1 HIGH HIGH HIGH = 16 microstep */
  HAL_GPIO_WritePin(A4988_MS1_GPIO_Port, A4988_MS1_Pin, 0);
  HAL_GPIO_WritePin(A4988_MS2_GPIO_Port, A4988_MS2_Pin, 0);
  HAL_GPIO_WritePin(A4988_MS3_GPIO_Port, A4988_MS3_Pin, 0);

  /* imposto step e direzione */
  HAL_GPIO_WritePin(A4988_DIR_GPIO_Port,  A4988_DIR_Pin,  0);
  HAL_GPIO_WritePin(A4988_STEP_GPIO_Port, A4988_STEP_Pin, 0);
  direction = 0; steps_to_do = 0;

  /* abilito il driver */
  HAL_GPIO_WritePin(A4988_nRST_GPIO_Port,   A4988_nRST_Pin,   1);
  HAL_GPIO_WritePin(A4988_nSLEEP_GPIO_Port, A4988_nSLEEP_Pin, 0);
  HAL_GPIO_WritePin(A4988_nEN_GPIO_Port,    A4988_nEN_Pin,    1);

  /* lettura device-id */
  int temp;
  device_id = -1;
  do {
	  temp = device_id;
	  HAL_Delay(100);
	  device_id = HAL_GPIO_ReadPin(ADD7_GPIO_Port, ADD7_Pin) & 0x01;
	  device_id = device_id << 1;
	  device_id += HAL_GPIO_ReadPin(ADD6_GPIO_Port, ADD6_Pin) & 0x01;
	  device_id = device_id << 1;
	  device_id += HAL_GPIO_ReadPin(ADD5_GPIO_Port, ADD5_Pin) & 0x01;
	  device_id = device_id << 1;
	  device_id += HAL_GPIO_ReadPin(ADD4_GPIO_Port, ADD4_Pin) & 0x01;
	  device_id = device_id << 1;
	  device_id += HAL_GPIO_ReadPin(ADD3_GPIO_Port, ADD3_Pin) & 0x01;
	  device_id = device_id << 1;
	  device_id += HAL_GPIO_ReadPin(ADD2_GPIO_Port, ADD2_Pin) & 0x01;
	  device_id = device_id << 1;
	  device_id += HAL_GPIO_ReadPin(ADD1_GPIO_Port, ADD1_Pin) & 0x01;
	  device_id = device_id << 1;
	  device_id += HAL_GPIO_ReadPin(ADD0_GPIO_Port, ADD0_Pin) & 0x01;
  } while (temp != device_id);


  // attiva la ricezione ad interrupt
  rxIndex = 0;
  EN_RX_485();
  HAL_Delay(10);
  HAL_UART_Receive_IT(&huart1, &rxRcvd, 1);

  // Avvia la conversione
  if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)anValues, 3) != HAL_OK) { Error_Handler(); }

  // invio stringa di debug
  sprintf((char *)txBuffer, "\r\n" VERSION "\r\n");
  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  status = HAL_ADC_GetState(&hadc1);

	  if ( (status & HAL_ADC_STATE_REG_BUSY) == 0 ) {

		  // La scansione ADC è completa
		  angolo4096 = (anValues[0] + anValues[1] + anValues[2]) / 3;

		  /*
		   // mostra il valore rilevato
		   sprintf((char *)txBuffer, "%5d %5d %5d : %06d\r"
				    , anValues[0], anValues[1], anValues[2]
				    , angolo4096);
		   HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);
		   */

		  // Riavvia un'altra conversione
		  if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)anValues, 3) != HAL_OK) { Error_Handler(); }

		  // se devo posizionare ad anello chiuso,
		  // eseguo la movimentazione
		  if (deg4096_to_go >= 0) {

 			  // abilitazione
 			  HAL_GPIO_WritePin(A4988_nSLEEP_GPIO_Port, A4988_nSLEEP_Pin, 1);
 			  HAL_GPIO_WritePin(A4988_nEN_GPIO_Port,    A4988_nEN_Pin,    0);
 			  /* A4988 MS3, MS2, MS1 LOW  HIGH HIGH =  8 microstep */
 			  HAL_GPIO_WritePin(A4988_MS1_GPIO_Port, A4988_MS1_Pin, 1);
 			  HAL_GPIO_WritePin(A4988_MS2_GPIO_Port, A4988_MS2_Pin, 1);
 			  HAL_GPIO_WritePin(A4988_MS3_GPIO_Port, A4988_MS3_Pin, 0);
 			  HAL_Delay(1);

 			  // vai avanti se è troppo indietro
 			  if (angolo4096 < (deg4096_to_go - SAFETY_GAP)) {
 				  HAL_GPIO_WritePin(A4988_DIR_GPIO_Port,  A4988_DIR_Pin, 1);
 			  }
 			  else
 	 		  // vai indietro se è troppo avanti
 	 		  if (angolo4096 > (deg4096_to_go + SAFETY_GAP)) {
 	 			  HAL_GPIO_WritePin(A4988_DIR_GPIO_Port,  A4988_DIR_Pin, 0);
 	 		  }
 	 		  else
 	 	      // sono arrivato dentro la banda di sicurezza +/- SAFETY_GAP
 			  //: disabilito
 	 		  {
 	 			  deg4096_to_go = -1;

 	 			  // disabilitazione
 	 			  HAL_GPIO_WritePin(A4988_nSLEEP_GPIO_Port, A4988_nSLEEP_Pin, 0);
 	 			  HAL_GPIO_WritePin(A4988_nEN_GPIO_Port,    A4988_nEN_Pin,    1);
 	 		  }

 			  /* faccio uno step */
 			  HAL_GPIO_WritePin(A4988_STEP_GPIO_Port, A4988_STEP_Pin, 1);
 			  HAL_Delay(1);
 			  HAL_GPIO_WritePin(A4988_STEP_GPIO_Port, A4988_STEP_Pin, 0);
 			  HAL_Delay(1);

		  }
	  }

	  /* controllo del motore */

	  /* protocollo JSON */
	  long int json1 = ParserJSON("id");
	  long int json2 = ParserJSON("dir");
	  long int json3 = ParserJSON("step");
	  long int json4 = ParserJSON("ppr");   // pulses per revolution
	  long int json5 = ParserJSON("deg");   // eseguo subito un posizionamento con controllo ad anello chiuso
	  long int json6 = ParserJSON("go");
	  long int json7 = ParserJSON("rnd");   // serve a complicare il protocollo e renderlo un po' più robusto
	  long int csum  = ParserJSON("csum");
 	  int rnd = HAL_GetTick() & 0xffff;     // genera numero pseudo casuale


	  if ((json1 == device_id) || (json1 == DEVICE_ID_BROADCAST)) {

		  // attendo un attimo per consentire lo switch TX->RX al master
		  HAL_Delay(1);

		  // invio stringa di debug
		  //sprintf((char *)txBuffer, "<-- %s\n\r", rxBuffer);
		  //HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);

		  // FF = Broadcasting

		  long int tmpCsum = json1;
		  if ( json2 != JSON_TOKEN_NOT_FOUND ) { tmpCsum += json2; }
		  if ( json3 != JSON_TOKEN_NOT_FOUND ) { tmpCsum += json3; }
		  if ( json4 != JSON_TOKEN_NOT_FOUND ) { tmpCsum += json4; }
		  if ( json5 != JSON_TOKEN_NOT_FOUND ) { tmpCsum += json5; }
		  if ( json6 != JSON_TOKEN_NOT_FOUND ) { tmpCsum += json6; }
		  if ( json7 != JSON_TOKEN_NOT_FOUND ) { tmpCsum += json7; }

	 	  if ( csum == tmpCsum ) {

	 		  // invio stringa di debug
	 		  //sprintf((char *)txBuffer, " ID=%d (mio ID=%d)\n\r", json1, device_id);
	 		  //HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);

	 		  // esegue il comando

	 		  // PULSES PER REVOLUTION
	 		  if ( json4 != JSON_TOKEN_NOT_FOUND ) {
	 			  switch ( json4 ) {
 			  	  	  case 400: /* A4988 MS3, MS2, MS1 LOW  LOW  HIGH =   HALF STEP  */
 			  	  		  HAL_GPIO_WritePin(A4988_MS1_GPIO_Port, A4988_MS1_Pin, 1);
 			  	  		  HAL_GPIO_WritePin(A4988_MS2_GPIO_Port, A4988_MS2_Pin, 0);
 			  	  		  HAL_GPIO_WritePin(A4988_MS3_GPIO_Port, A4988_MS3_Pin, 0);
 			  	  		  break;

 			  	  	  case 800: /* A4988 MS3, MS2, MS1 LOW  HIGH LOW  =  4 microstep */
 			  	  		  HAL_GPIO_WritePin(A4988_MS1_GPIO_Port, A4988_MS1_Pin, 0);
 			  	  		  HAL_GPIO_WritePin(A4988_MS2_GPIO_Port, A4988_MS2_Pin, 1);
 			  	  		  HAL_GPIO_WritePin(A4988_MS3_GPIO_Port, A4988_MS3_Pin, 0);
 			  	  		  break;

 			  	  	  case 1600: /* A4988 MS3, MS2, MS1 LOW  HIGH HIGH =  8 microstep */
 			  	  		  HAL_GPIO_WritePin(A4988_MS1_GPIO_Port, A4988_MS1_Pin, 1);
 			  	  		  HAL_GPIO_WritePin(A4988_MS2_GPIO_Port, A4988_MS2_Pin, 1);
 			  	  		  HAL_GPIO_WritePin(A4988_MS3_GPIO_Port, A4988_MS3_Pin, 0);
 			  	  		  break;

 			  	  	  case 3200: /* A4988 MS3, MS2, MS1 HIGH HIGH HIGH = 16 microstep */
 			  	  		  HAL_GPIO_WritePin(A4988_MS1_GPIO_Port, A4988_MS1_Pin, 1);
 			  	  		  HAL_GPIO_WritePin(A4988_MS2_GPIO_Port, A4988_MS2_Pin, 1);
 			  	  		  HAL_GPIO_WritePin(A4988_MS3_GPIO_Port, A4988_MS3_Pin, 1);
 			  	  		  break;

	 			  	  case 200:   /* A4988 MS3, MS2, MS1 LOW  LOW  LOW  =   FULL STEP  */
	 			  	  default:
	 			  		  HAL_GPIO_WritePin(A4988_MS1_GPIO_Port, A4988_MS1_Pin, 0);
	 			  		  HAL_GPIO_WritePin(A4988_MS2_GPIO_Port, A4988_MS2_Pin, 0);
	 			  		  HAL_GPIO_WritePin(A4988_MS3_GPIO_Port, A4988_MS3_Pin, 0);
	 			  		  break;
	 			  }
	 		  }

	 		  // DIRECTION
	 		  if ( json2 != JSON_TOKEN_NOT_FOUND ) {
	 			 direction = (json2 != 0)?(1):(-1);  // 1 avanti, -1 indietro
	 		  }

	 		  // STEPS
	 		  if ( json3 != JSON_TOKEN_NOT_FOUND ) {
	 			 steps_to_do += direction * json3;
	 		  }

	 		  // DEG (eseguo subito)
	 		  if ( json5 != JSON_TOKEN_NOT_FOUND ) {
	 			  deg4096_to_go = json5;
	 		  }


	 		  // GO
	 		  if (( json6 != JSON_TOKEN_NOT_FOUND ) && ( json6 == 1 )) {

	 			  // abilitazione
	 			  HAL_GPIO_WritePin(A4988_nSLEEP_GPIO_Port, A4988_nSLEEP_Pin, 1);
	 			  HAL_GPIO_WritePin(A4988_nEN_GPIO_Port,    A4988_nEN_Pin,    0);
	 			  HAL_Delay(10);

	 			  // vai avanti
	 			  while (steps_to_do > 0) {
	 				  HAL_GPIO_WritePin(A4988_DIR_GPIO_Port,  A4988_DIR_Pin, 1);

		 			  /* faccio uno step */
		 			  HAL_GPIO_WritePin(A4988_STEP_GPIO_Port, A4988_STEP_Pin, 1);
		 			  HAL_Delay(1);
		 			  HAL_GPIO_WritePin(A4988_STEP_GPIO_Port, A4988_STEP_Pin, 0);
		 			  HAL_Delay(1);

		 			  steps_to_do --;
	 			  }

	 			  // vai indietro
	 			  while (steps_to_do < 0) {
	 				  HAL_GPIO_WritePin(A4988_DIR_GPIO_Port,  A4988_DIR_Pin, 0);

		 			  /* faccio uno step */
		 			  HAL_GPIO_WritePin(A4988_STEP_GPIO_Port, A4988_STEP_Pin, 1);
		 			  HAL_Delay(1);
		 			  HAL_GPIO_WritePin(A4988_STEP_GPIO_Port, A4988_STEP_Pin, 0);
		 			  HAL_Delay(1);

		 			  steps_to_do ++;
	 			  }

	 			  // disabilitazione
 	 			  HAL_GPIO_WritePin(A4988_nSLEEP_GPIO_Port, A4988_nSLEEP_Pin, 0);
	 			  HAL_GPIO_WritePin(A4988_nEN_GPIO_Port,    A4988_nEN_Pin,    1);

	 		  }

	 		  if (json1 == device_id) {
	 			  // non risponde per i broadcasting

	 			  EN_TX_485();

	 			  // preparo la stringa con due byte di stuffing avanti e indietro
	 			  sprintf((char *)txBuffer, "  {id:%d,deg:%d,ack:1,rnd:%d,csum:%d}\n ",
	 	  				              device_id, angolo4096, rnd, device_id + angolo4096 + 1 + rnd );

	 			  // se Timeout != 0 la Transmit è bloccante
	 			  HAL_UART_Transmit(&huart1,  txBuffer, strlen((char *)txBuffer), 100);

	 			  EN_RX_485();
	 		  }
	 	  }
	 	  else if (json1 == device_id) {
 			  // risponde solo se è diretto a me e non per i broadcasting

	 		  EN_TX_485();

	 		  // preparo la stringa con due byte di stuffing avanti e indietro
 			  sprintf((char *)txBuffer, "  {id:%d,ack:0,rnd:%d,csum:%d}\n ",
 	  				              device_id, rnd, device_id + 1 + rnd );

 			  // se Timeout != 0 la Transmit è bloccante
 			  HAL_UART_Transmit(&huart1,  txBuffer, strlen((char *)txBuffer), 100);

 			  EN_RX_485();

	 	  }

	 	  // riavvia la ricezione per un'altra stringa
	 	  memset(rxBuffer, 0, RX_BUFFER_SIZE);
	 	  rxIndex = 0;
	  }

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the common peripherals clocks
  */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_HSIKER;
  PeriphClkInit.HSIKerClockDivider = RCC_HSIKER_DIV4;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.LowPowerAutoPowerOff = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 3;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_39CYCLES_5;
  hadc1.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_39CYCLES_5;
  hadc1.Init.OversamplingMode = DISABLE;
  hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* Calibrazione ADC */
  HAL_ADCEx_Calibration_Start(&hadc1);

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 19200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_RXOVERRUNDISABLE_INIT;
  huart1.AdvancedInit.OverrunDisable = UART_ADVFEATURE_OVERRUN_DISABLE;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_8_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_EnableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 19200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_RXOVERRUNDISABLE_INIT|UART_ADVFEATURE_DMADISABLEONERROR_INIT;
  huart2.AdvancedInit.OverrunDisable = UART_ADVFEATURE_OVERRUN_DISABLE;
  huart2.AdvancedInit.DMADisableonRxError = UART_ADVFEATURE_DMA_DISABLEONRXERROR;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, A4988_nEN_Pin|A4988_MS1_Pin|A4988_MS2_Pin|A4988_MS3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, A4988_nRST_Pin|A4988_nSLEEP_Pin|A4988_STEP_Pin|A4988_DIR_Pin
                          |U1_TXEN_485_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : A4988_nEN_Pin A4988_MS1_Pin A4988_MS2_Pin A4988_MS3_Pin */
  GPIO_InitStruct.Pin = A4988_nEN_Pin|A4988_MS1_Pin|A4988_MS2_Pin|A4988_MS3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : A4988_nRST_Pin A4988_nSLEEP_Pin A4988_STEP_Pin A4988_DIR_Pin
                           U1_TXEN_485_Pin */
  GPIO_InitStruct.Pin = A4988_nRST_Pin|A4988_nSLEEP_Pin|A4988_STEP_Pin|A4988_DIR_Pin
                          |U1_TXEN_485_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : ADD0_Pin ADD1_Pin ADD2_Pin */
  GPIO_InitStruct.Pin = ADD0_Pin|ADD1_Pin|ADD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : ADD3_Pin ADD4_Pin ADD7_Pin */
  GPIO_InitStruct.Pin = ADD3_Pin|ADD4_Pin|ADD7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : ADD5_Pin ADD6_Pin */
  GPIO_InitStruct.Pin = ADD5_Pin|ADD6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1)
    {
    	rxBuffer[rxIndex] = rxRcvd;

        // il carattere dovrebbe essere già stato letto da HAL in rxBuffer[rxIndex]
    	if (rxBuffer[0] == '{') {

    		/* è partito un JSON */
    		rxIndex ++;
    		if (rxIndex < RX_BUFFER_SIZE) {
    			/* termino sempre la stringa  per sicurezza */
    			rxBuffer[rxIndex] = '\0';
    		}
    		else {
    			/* si è riempito il buffer, non va bene, resetto tutto e riparto */
    			memset(rxBuffer, 0, RX_BUFFER_SIZE);
    			rxIndex = 0;
    		}
    	}
    	else {
    	    rxIndex = 0;
    	}

        // riattivo la ricezione
    	HAL_UART_Receive_IT(&huart1, &rxRcvd, 1);
    }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();

  sprintf((char *)txBuffer, "Error! %s %d\n\r", __FILE__,__LINE__);
  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);


  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
