/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
	False = 0,
	True  = 1
}type_bool;

typedef enum {     
	Detecting           = 0,
	Possible_Transition = 1,
	Detected            = 2
}type_detection;

typedef struct
{
	uint32_t initial_time;
	uint32_t elapsed_time;
	uint32_t delay_time;
}type_ST;
typedef struct {
	type_ST timer;
	GPIO_TypeDef* port_address;
	uint16_t pin;
	uint32_t period_ms;
	uint32_t duty_ms;
	type_bool state;
	uint32_t shadow_duty;
	uint32_t shadow_period;
}type_PWM;

typedef struct {
	GPIO_TypeDef* port_address;
	uint16_t pin;
	type_bool current_state;
	type_bool previous_state;
	type_ST debounce_timer;
	uint32_t debounce_time;
	type_detection state;
}button_handler;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void ST_init(type_ST *pST, uint32_t time_lapse);
type_bool ST(type_ST *pST);
void ST_Lapse(type_ST *pST);

void PWM_init(type_PWM *PWM, GPIO_TypeDef* port, uint32_t pin, uint32_t used_period, uint32_t desired_duty);
void PWM_run(type_PWM *PWM);
void PWM_update(type_PWM *PWM, uint32_t period, uint32_t duty, type_bool update_now);

// Funções de button handling
void button_init(button_handler* aBH, uint32_t d_time, GPIO_TypeDef* pport, uint16_t pin);
type_bool border_detection(button_handler* aBH, type_bool button);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	/* USER CODE BEGIN 2 */
	HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED_O_GPIO_Port, LED_O_Pin, GPIO_PIN_SET);
	
	// Váriaveis de timer
	type_ST timerG;
	type_ST timerO;
	type_PWM PWMorange;
	
	PWM_init(&PWMorange, LED_O_GPIO_Port, LED_O_Pin, 2000, 10);
	ST_init(&timerG, 2000);
	ST_init(&timerO, 2000);
	
	type_bool j = True;
	type_bool i = False; // usado para o LED Azul
	
	// Variáveis para borda
	type_bool but_status = True;
	type_bool but_memory = True;
	HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_RESET);
	type_ST timer_debounce;
	ST_init(&timer_debounce, 100);
#define DEBOUNCE_V4
	type_detection BUT_B_State = Detecting;
	
	
	// teste de handler
	button_handler BLUE_BUT;
	button_init(&BLUE_BUT, 50, BOT_B_GPIO_Port, BOT_B_Pin);
		
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		PWM_run(&PWMorange);
		
		if (ST(&timerG)) {
			ST_Lapse(&timerG);
			HAL_GPIO_TogglePin(LED_G_GPIO_Port, LED_G_Pin);
		}  
		if (ST(&timerO)) {
			ST_Lapse(&timerO);
			if (j) {
				j = False;
				PWM_update(&PWMorange, 2000, 10, 0);
			}
			else {
				j = True;
				PWM_update(&PWMorange, 2000, 90, 0);
			}
		}  

		but_status = (type_bool)HAL_GPIO_ReadPin(BOT_B_GPIO_Port, BOT_B_Pin);
		// Rotina para LED Azul ----------------------------------------------------------------------------
		if (but_status && !i) {
			HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin, GPIO_PIN_SET);
			i = 1;
		}	
		else if (i) {
			HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin, GPIO_PIN_RESET);
			i = 0;
		}
	
		// Rotina para detecção de borda -------------------------------------------------------------------

#ifdef DEBOUNCE_V1
				
		but_memory = but_status;
		but_status = (type_bool)HAL_GPIO_ReadPin(BOT_B_GPIO_Port, BOT_B_Pin);
		if (!but_memory && but_status && ST(&timer_debounce)) {
			// Borda de subida 
			HAL_GPIO_TogglePin(LED_R_GPIO_Port, LED_R_Pin);
			ST_Init(&timer_debounce, 100);
		}
		else if (but_memory && !but_status) {
			// borda de descida 
			ST_Init(&timer_debounce, 100);
		}
#endif 

#ifdef DEBOUNCE_V2
		
		but_memory = but_status;
		but_status = (type_bool)HAL_GPIO_ReadPin(BOT_B_GPIO_Port, BOT_B_Pin);
		if (ST(&timer_debounce) && but_status != but_memory) {
			ST_init(&timer_debounce, 50);
			if (!but_memory && but_status) {
				HAL_GPIO_TogglePin(LED_R_GPIO_Port, LED_R_Pin);
			}
		}
#endif
#ifdef DEBOUNCE_V3
		if (border_detection(&BLUE_BUT)) {
			HAL_GPIO_TogglePin(LED_R_GPIO_Port, LED_R_Pin);
		}		  
#endif
#ifdef DEBOUNCE_V4	  
		if (border_detection(&BLUE_BUT, but_status)) {
			HAL_GPIO_TogglePin(LED_R_GPIO_Port, LED_R_Pin);
		}
#endif // DEBOUNCE_V4
	}
	/* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	*/
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	* in the RCC_OscInitTypeDef structure.
	*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 168;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
	                            | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
	{
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */
void ST_init(type_ST *pST, uint32_t time_lapse) {
	pST->delay_time = time_lapse;
	pST->initial_time = HAL_GetTick();
	pST->elapsed_time = 0;
}

type_bool ST(type_ST *pST) {
	pST->elapsed_time = HAL_GetTick() - pST->initial_time;
	return (pST->elapsed_time >= pST->delay_time);
}

void ST_Lapse(type_ST *pST) {
	pST->initial_time += pST->delay_time;
	pST->elapsed_time = 0;
}

void PWM_init(type_PWM *PWM, GPIO_TypeDef* port, uint32_t pin, uint32_t used_period, uint32_t desired_duty) {
	if (desired_duty > 100)
	{
		desired_duty = 100;
	}
	
	PWM->period_ms = used_period;
	PWM->duty_ms = used_period * desired_duty / 100;
	PWM->pin = pin;
	PWM->port_address = port;
	PWM->shadow_duty = PWM->duty_ms;
	PWM->shadow_period = PWM->period_ms;
	
	ST_init(&PWM->timer, PWM->duty_ms);
	HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
	PWM->state = True;
}

void PWM_run(type_PWM *PWM) {
	if (ST(&PWM->timer)) {
		ST_Lapse(&PWM->timer);
		if (PWM->state) {
			PWM->state = False;
			HAL_GPIO_WritePin(PWM->port_address, PWM->pin, GPIO_PIN_RESET);
			PWM->timer.delay_time = PWM->period_ms - PWM->duty_ms;
		}
		else {
			PWM->state = True;
			HAL_GPIO_WritePin(PWM->port_address, PWM->pin, GPIO_PIN_SET);
			
			// Aqui ele atualiza o Shadow, i.e., garante que a alteração só ocorre ao fim de um período
			PWM->duty_ms = PWM->shadow_duty; 
			PWM->period_ms = PWM->shadow_period;
			
			PWM->timer.delay_time = PWM->duty_ms;
		}
	}
}

void PWM_update(type_PWM *PWM, uint32_t period, uint32_t desired_duty, type_bool update_now) {
	if (desired_duty > 100)
	{
		desired_duty = 100;
	}
	PWM->shadow_period = period;
	PWM->shadow_duty = desired_duty * period / 100;
	if (update_now) {
		PWM->duty_ms = PWM->shadow_duty; 
		PWM->period_ms = PWM->shadow_period;
	}
}

// Funções de Handler de Botão ------------------------------------------------------
void button_init(button_handler* aBH, uint32_t d_time, GPIO_TypeDef* pport, uint16_t pin) {
	aBH->port_address = pport;
	aBH->pin = pin;
	aBH->current_state = (type_bool)HAL_GPIO_ReadPin(aBH->port_address, aBH->pin);
	aBH->previous_state = aBH->current_state;
	aBH->debounce_time = d_time;
	aBH->state = Detecting;
	ST_init(&aBH->debounce_timer, d_time);
}

type_bool border_detection(button_handler * aBH, type_bool button) {
	aBH->current_state = button;
	switch (aBH->state) {
	case Detecting:
		if (aBH->current_state && !aBH->previous_state) {
			aBH->state = Possible_Transition;
			ST_init(&aBH->debounce_timer, aBH->debounce_time);
		}
		aBH->previous_state = aBH->current_state;
		return 0;
		break;
	case Possible_Transition:
		if (ST(&aBH->debounce_timer)) {
			if (aBH->current_state) {
				aBH->state = Detected;
			}
			else {
				aBH->state = Detecting;
			}
		}
		return 0;
		break;
	case Detected:
		aBH->state = Detecting;
		return 1;
		break;
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
	while (1)
	{
	}
	/* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
