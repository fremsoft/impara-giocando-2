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
#define DEVICE_ID_BROADCAST    0x100

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
volatile uint8_t rxIndex, rxRcvd, rxBuffer[RX_BUFFER_SIZE];

uint16_t anValues[3], angolo4096;

#define NO_COMMAND '.'
volatile uint8_t command;


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

	int device_id;

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

  /* abilito il driver */
  HAL_GPIO_WritePin(A4988_nRST_GPIO_Port,   A4988_nRST_Pin,   1);
  HAL_GPIO_WritePin(A4988_nSLEEP_GPIO_Port, A4988_nSLEEP_Pin, 1);
  HAL_GPIO_WritePin(A4988_nEN_GPIO_Port,    A4988_nEN_Pin,    0);

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


  // attiva la trasmissione su RS485

  EN_TX_485();
  HAL_Delay(10);

  // attiva la ricezione su RS485
  //EN_RX_485();
  rxIndex = 0;
  HAL_UART_Receive_IT(&huart1, &rxRcvd, 1);

  // attiva la ricezione sulla seriale di servizio
  HAL_UART_Receive_IT(&huart2, &rxRcvd, 1);


  // Avvia la conversione
  if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)anValues, 3) != HAL_OK) { Error_Handler(); }

  // invio stringa di debug
  sprintf((char *)txBuffer, "\r\n" VERSION "\r\n");
  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);


  command = NO_COMMAND;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  /* invio stringa per eseguire un comando */
	  static long int json1 = 1;         // id
	  static long int json2 = 1;         // dir
	  static long int json3 = 0;         // step
	  static long int json4 = 200 * 8;   // pulses per revolution
	  static long int json5 = 0;         // deg 4096 closed loop positioning
	  static long int json6 = 0;         // go
	  long int json7;
	  long int csum;

	  json7 = HAL_GetTick() & 0xffff; // genera numero pseudo casuale

	  // invio stringa a DRIVER

	  if (command == '1') {

		  // trasmissione debug string
		  sprintf((char *)txBuffer, "--> ");
		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);

		  json2 = 1;     // direzione avanti
		  json3 = 1000;  // passi
		  csum = json1+json2+json3+json4+json7;
		  sprintf((char *)txBuffer, "     {id:%ld,dir:%ld,step:%ld,ppr:%ld,rnd:%ld,csum:%ld}"
 			 , json1 /* id */
			 , json2 /* dir */
			 , json3 /* step */
			 , json4 /* ppr */
			 , json7, csum);
		  EN_TX_485();   // IDLE VCC
		  // se Timeout != 0 la Transmit è bloccante
		  HAL_UART_Transmit(&huart1,  txBuffer, strlen((char *)txBuffer), 100);
		  EN_RX_485();
		  HAL_Delay(100);

		  // trasmissione debug string
		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);
		  sprintf((char *)txBuffer, "\r\n<-- %s\r\n", rxBuffer);
		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);
		  memset((uint8_t *)rxBuffer, 0, RX_BUFFER_SIZE);
		  rxIndex = 0;

		  command = NO_COMMAND;
	  }

	  if (command == '2') {

		  // trasmissione debug string
		  sprintf((char *)txBuffer, "--> ");
		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);

		  json2 = 1;
		  json3 = 600;
		  csum = json1+json2+json3+json4+json7;
		  sprintf((char *)txBuffer, "     {id:%ld,dir:%ld,step:%ld,ppr:%ld,rnd:%ld,csum:%ld}"
 			 , json1 /* id */
			 , json2 /* dir */
			 , json3 /* step */
			 , json4 /* ppr */
			 , json7, csum);
		  EN_TX_485();   // IDLE VCC
		  // se Timeout != 0 la Transmit è bloccante
		  HAL_UART_Transmit(&huart1,  txBuffer, strlen((char *)txBuffer), 100);
		  EN_RX_485();
		  HAL_Delay(100);

		  // trasmissione debug string
		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);
		  sprintf((char *)txBuffer, "\r\n<-- %s\r\n", rxBuffer);
		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);
		  memset((uint8_t *)rxBuffer, 0, RX_BUFFER_SIZE);
		  rxIndex = 0;

		  command = NO_COMMAND;
	  }

	  if (command == '3') {

		  // trasmissione debug string
		  sprintf((char *)txBuffer, "--> ");
		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);

		  json2 = 0;
		  json3 = 1000;
		  csum = json1+json2+json3+json4+json7;
		  sprintf((char *)txBuffer, "     {id:%ld,dir:%ld,step:%ld,ppr:%ld,rnd:%ld,csum:%ld}"
 			 , json1 /* id */
			 , json2 /* dir */
			 , json3 /* step */
			 , json4 /* ppr */
			 , json7, csum);
		  EN_TX_485();   // IDLE VCC
		  // se Timeout != 0 la Transmit è bloccante
		  HAL_UART_Transmit(&huart1,  txBuffer, strlen((char *)txBuffer), 100);
		  EN_RX_485();
		  HAL_Delay(100);

		  // trasmissione debug string
		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);
		  sprintf((char *)txBuffer, "\r\n<-- %s\r\n", rxBuffer);
		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);
		  memset((uint8_t *)rxBuffer, 0, RX_BUFFER_SIZE);
		  rxIndex = 0;

		  command = NO_COMMAND;
	  }

	  if (command == '4') {

		  // trasmissione debug string
		  sprintf((char *)txBuffer, "--> ");
		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);

		  json2 = 0;
		  json3 = 600;
		  csum = json1+json2+json3+json4+json7;
		  sprintf((char *)txBuffer, "     {id:%ld,dir:%ld,step:%ld,ppr:%ld,rnd:%ld,csum:%ld}"
 			 , json1 /* id */
			 , json2 /* dir */
			 , json3 /* step */
			 , json4 /* ppr */
			 , json7, csum);
		  EN_TX_485();   // IDLE VCC
		  // se Timeout != 0 la Transmit è bloccante
		  HAL_UART_Transmit(&huart1,  txBuffer, strlen((char *)txBuffer), 100);
		  EN_RX_485();
		  HAL_Delay(100);

		  // trasmissione debug string
		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);
		  sprintf((char *)txBuffer, "\r\n<-- %s\r\n", rxBuffer);
		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);
		  memset((uint8_t *)rxBuffer, 0, RX_BUFFER_SIZE);
		  rxIndex = 0;

		  command = NO_COMMAND;
	  }

	  if (command == '5') {

	  	  json4 = 200;
  	  	  // trasmissione debug string
	  	  sprintf((char *)txBuffer, "Set 200 PPR\r\n");
	  	  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);

	  	  command = NO_COMMAND;
	  }
	  if (command == '6') {

	  	  json4 = 2*200;
  	  	  // trasmissione debug string
	  	  sprintf((char *)txBuffer, "Set 400 PPR\r\n");
	  	  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);

	  	  command = NO_COMMAND;
	  }
	  if (command == '7') {

	  	  json4 = 4*200;
  	  	  // trasmissione debug string
	  	  sprintf((char *)txBuffer, "Set 800 PPR\r\n");
	  	  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);

	  	  command = NO_COMMAND;
	  }
	  if (command == '8') {

	  	  json4 = 8*200;
  	  	  // trasmissione debug string
	  	  sprintf((char *)txBuffer, "Set 1600 PPR\r\n");
	  	  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);

	  	  command = NO_COMMAND;
	  }
	  if (command == '9') {

	  	  json4 = 16*200;
  	  	  // trasmissione debug string
	  	  sprintf((char *)txBuffer, "Set 3200 PPR\r\n");
	  	  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);

	  	  command = NO_COMMAND;
	  }


	  if (command == 'g') {

		  // trasmissione debug string
		  sprintf((char *)txBuffer, "--> ");
		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);

		  json6 = 1;
		  csum = DEVICE_ID_BROADCAST+json6+json7;
		  sprintf((char *)txBuffer, "     {id:%d,go:%ld,rnd:%ld,csum:%ld}"
 			 , DEVICE_ID_BROADCAST /* broadcast */
			 , json6 /* go */
			 , json7, csum);
		  EN_TX_485();   // IDLE VCC
		  // se Timeout != 0 la Transmit è bloccante
		  HAL_UART_Transmit(&huart1,  txBuffer, strlen((char *)txBuffer), 100);
		  EN_RX_485();
		  HAL_Delay(100);

		  // trasmissione debug string
		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);
		  sprintf((char *)txBuffer, "\r\n");
		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);

		  command = NO_COMMAND;
	  }

	  if (command == 'w') {

	  		  // trasmissione debug string
	  		  sprintf((char *)txBuffer, "--> ");
	  		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);

	  		  json5 = 0;     // deg 4096
	  		  csum = json1+json5+json7;
	  		  sprintf((char *)txBuffer, "     {id:%ld,deg:%ld,rnd:%ld,csum:%ld}"
	   			 , json1 /* id */
	  			 , json5 /* dir */
	  			 , json7, csum);
	  		  EN_TX_485();   // IDLE VCC
	  		  // se Timeout != 0 la Transmit è bloccante
	  		  HAL_UART_Transmit(&huart1,  txBuffer, strlen((char *)txBuffer), 100);
	  		  EN_RX_485();
	  		  HAL_Delay(100);

	  		  // trasmissione debug string
	  		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);
	  		  sprintf((char *)txBuffer, "\r\n<-- %s\r\n", rxBuffer);
	  		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);
	  		  memset((uint8_t *)rxBuffer, 0, RX_BUFFER_SIZE);
	  		  rxIndex = 0;

	  		  command = NO_COMMAND;
	  	  }
	  if (command == 'e') {

	  		  // trasmissione debug string
	  		  sprintf((char *)txBuffer, "--> ");
	  		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);

	  		  json5 = 512;     // deg 4096
	  		  csum = json1+json5+json7;
	  		  sprintf((char *)txBuffer, "     {id:%ld,deg:%ld,rnd:%ld,csum:%ld}"
	   			 , json1 /* id */
	  			 , json5 /* dir */
	  			 , json7, csum);
	  		  EN_TX_485();   // IDLE VCC
	  		  // se Timeout != 0 la Transmit è bloccante
	  		  HAL_UART_Transmit(&huart1,  txBuffer, strlen((char *)txBuffer), 100);
	  		  EN_RX_485();
	  		  HAL_Delay(100);

	  		  // trasmissione debug string
	  		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);
	  		  sprintf((char *)txBuffer, "\r\n<-- %s\r\n", rxBuffer);
	  		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);
	  		  memset((uint8_t *)rxBuffer, 0, RX_BUFFER_SIZE);
	  		  rxIndex = 0;

	  		  command = NO_COMMAND;
	  	  }

	  if (command == 'd') {

	  		  // trasmissione debug string
	  		  sprintf((char *)txBuffer, "--> ");
	  		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);

	  		  json5 = 1024;     // deg 4096
	  		  csum = json1+json5+json7;
	  		  sprintf((char *)txBuffer, "     {id:%ld,deg:%ld,rnd:%ld,csum:%ld}"
	   			 , json1 /* id */
	  			 , json5 /* dir */
	  			 , json7, csum);
	  		  EN_TX_485();   // IDLE VCC
	  		  // se Timeout != 0 la Transmit è bloccante
	  		  HAL_UART_Transmit(&huart1,  txBuffer, strlen((char *)txBuffer), 100);
	  		  EN_RX_485();
	  		  HAL_Delay(100);

	  		  // trasmissione debug string
	  		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);
	  		  sprintf((char *)txBuffer, "\r\n<-- %s\r\n", rxBuffer);
	  		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);
	  		  memset((uint8_t *)rxBuffer, 0, RX_BUFFER_SIZE);
	  		  rxIndex = 0;

	  		  command = NO_COMMAND;
	  	  }
	  if (command == 'c') {

	  		  // trasmissione debug string
	  		  sprintf((char *)txBuffer, "--> ");
	  		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);

	  		  json5 = 1536;     // deg 4096
	  		  csum = json1+json5+json7;
	  		  sprintf((char *)txBuffer, "     {id:%ld,deg:%ld,rnd:%ld,csum:%ld}"
	   			 , json1 /* id */
	  			 , json5 /* dir */
	  			 , json7, csum);
	  		  EN_TX_485();   // IDLE VCC
	  		  // se Timeout != 0 la Transmit è bloccante
	  		  HAL_UART_Transmit(&huart1,  txBuffer, strlen((char *)txBuffer), 100);
	  		  EN_RX_485();
	  		  HAL_Delay(100);

	  		  // trasmissione debug string
	  		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);
	  		  sprintf((char *)txBuffer, "\r\n<-- %s\r\n", rxBuffer);
	  		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);
	  		  memset((uint8_t *)rxBuffer, 0, RX_BUFFER_SIZE);
	  		  rxIndex = 0;

	  		  command = NO_COMMAND;
	  	  }
	  if (command == 'x') {

	  		  // trasmissione debug string
	  		  sprintf((char *)txBuffer, "--> ");
	  		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);

	  		  json5 = 2048;     // deg 4096
	  		  csum = json1+json5+json7;
	  		  sprintf((char *)txBuffer, "     {id:%ld,deg:%ld,rnd:%ld,csum:%ld}"
	   			 , json1 /* id */
	  			 , json5 /* dir */
	  			 , json7, csum);
	  		  EN_TX_485();   // IDLE VCC
	  		  // se Timeout != 0 la Transmit è bloccante
	  		  HAL_UART_Transmit(&huart1,  txBuffer, strlen((char *)txBuffer), 100);
	  		  EN_RX_485();
	  		  HAL_Delay(100);

	  		  // trasmissione debug string
	  		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);
	  		  sprintf((char *)txBuffer, "\r\n<-- %s\r\n", rxBuffer);
	  		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);
	  		  memset((uint8_t *)rxBuffer, 0, RX_BUFFER_SIZE);
	  		  rxIndex = 0;

	  		  command = NO_COMMAND;
	  	  }
	  if (command == 'z') {

	  		  // trasmissione debug string
	  		  sprintf((char *)txBuffer, "--> ");
	  		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);

	  		  json5 = 2560;     // deg 4096
	  		  csum = json1+json5+json7;
	  		  sprintf((char *)txBuffer, "     {id:%ld,deg:%ld,rnd:%ld,csum:%ld}"
	   			 , json1 /* id */
	  			 , json5 /* dir */
	  			 , json7, csum);
	  		  EN_TX_485();   // IDLE VCC
	  		  // se Timeout != 0 la Transmit è bloccante
	  		  HAL_UART_Transmit(&huart1,  txBuffer, strlen((char *)txBuffer), 100);
	  		  EN_RX_485();
	  		  HAL_Delay(100);

	  		  // trasmissione debug string
	  		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);
	  		  sprintf((char *)txBuffer, "\r\n<-- %s\r\n", rxBuffer);
	  		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);
	  		  memset((uint8_t *)rxBuffer, 0, RX_BUFFER_SIZE);
	  		  rxIndex = 0;

	  		  command = NO_COMMAND;
	  	  }
	  if (command == 'a') {

	  		  // trasmissione debug string
	  		  sprintf((char *)txBuffer, "--> ");
	  		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);

	  		  json5 = 3072;     // deg 4096
	  		  csum = json1+json5+json7;
	  		  sprintf((char *)txBuffer, "     {id:%ld,deg:%ld,rnd:%ld,csum:%ld}"
	   			 , json1 /* id */
	  			 , json5 /* dir */
	  			 , json7, csum);
	  		  EN_TX_485();   // IDLE VCC
	  		  // se Timeout != 0 la Transmit è bloccante
	  		  HAL_UART_Transmit(&huart1,  txBuffer, strlen((char *)txBuffer), 100);
	  		  EN_RX_485();
	  		  HAL_Delay(100);

	  		  // trasmissione debug string
	  		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);
	  		  sprintf((char *)txBuffer, "\r\n<-- %s\r\n", rxBuffer);
	  		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);
	  		  memset((uint8_t *)rxBuffer, 0, RX_BUFFER_SIZE);
	  		  rxIndex = 0;

	  		  command = NO_COMMAND;
	  	  }
	  if (command == 'q') {

	  		  // trasmissione debug string
	  		  sprintf((char *)txBuffer, "--> ");
	  		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);

	  		  json5 = 3584;     // deg 4096
	  		  csum = json1+json5+json7;
	  		  sprintf((char *)txBuffer, "     {id:%ld,deg:%ld,rnd:%ld,csum:%ld}"
	   			 , json1 /* id */
	  			 , json5 /* dir */
	  			 , json7, csum);
	  		  EN_TX_485();   // IDLE VCC
	  		  // se Timeout != 0 la Transmit è bloccante
	  		  HAL_UART_Transmit(&huart1,  txBuffer, strlen((char *)txBuffer), 100);
	  		  EN_RX_485();
	  		  HAL_Delay(100);

	  		  // trasmissione debug string
	  		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);
	  		  sprintf((char *)txBuffer, "\r\n<-- %s\r\n", rxBuffer);
	  		  HAL_UART_Transmit(&huart2,  txBuffer, strlen((char *)txBuffer), 100);
	  		  memset((uint8_t *)rxBuffer, 0, RX_BUFFER_SIZE);
	  		  rxIndex = 0;

	  		  command = NO_COMMAND;
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
    			memset((uint8_t *)rxBuffer, 0, RX_BUFFER_SIZE);
    			rxIndex = 0;
    		}
    	}
    	else {
    	    rxIndex = 0;
    	}

        // riattivo la ricezione
    	HAL_UART_Receive_IT(&huart1, &rxRcvd, 1);
    }

    if (huart == &huart2)
    {
    	command = rxRcvd;

        // riattivo la ricezione
    	HAL_UART_Receive_IT(&huart2, &rxRcvd, 1);
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
