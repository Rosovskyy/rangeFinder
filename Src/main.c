
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"
#include "lcd5110.h"


#define LOOP_FREQ (SystemCoreClock/4000000)
#define USE_TIM10_TIMING 1

/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */

volatile uint32_t tim10_overflows = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if( htim->Instance == TIM10 )
  {
   ++tim10_overflows;
  }
}

inline void TIM10_reinit()
{
 HAL_TIM_Base_Stop(&htim10);
 __HAL_TIM_SET_PRESCALER( &htim10, (SystemCoreClock/1000000) );
 __HAL_TIM_SET_COUNTER( &htim10, 0 );
 tim10_overflows = 0;
 HAL_TIM_Base_Start_IT(&htim10);
}

inline uint32_t get_tim10_us()
{
 __HAL_TIM_DISABLE_IT(&htim10, TIM_IT_UPDATE); //! Дуже важливо!
 //__disable_irq();
 uint32_t res = tim10_overflows * 10000 + __HAL_TIM_GET_COUNTER(&htim10);
 //__enable_irq();
 __HAL_TIM_ENABLE_IT(&htim10, TIM_IT_UPDATE);
 return res;
}

inline void udelay_TIM10(uint32_t useconds) {
 uint32_t before = get_tim10_us();
 while( get_tim10_us() < before+useconds){}
}

void init_timing()
{
#ifdef  USE_HAL_DELAY_AND_ASM
 return;
#elif defined USE_DWT_TIMING
 DWT_Init();
#elif defined USE_TIM10_TIMING
 TIM10_reinit();
#else
#error "Unknown timing method."
#endif
}

inline uint32_t get_us()
{
#ifdef  USE_HAL_DELAY_AND_ASM
 return 1000*HAL_GetTick();// ДУже грубо, а що зробиш?
#elif defined USE_DWT_TIMING
 return get_DWT_us();
#elif defined USE_TIM10_TIMING
 return get_tim10_us();
#else
#error "Unknown timing method."
#endif
}

inline void udelay(uint32_t useconds)
{
#ifdef  USE_HAL_DELAY_AND_ASM
 udelay_asm(useconds);
#elif defined USE_DWT_TIMING
 udelay_DWT(useconds);
#elif defined USE_TIM10_TIMING
 udelay_TIM10(useconds);
#else
#error "Unknown timing method."
#endif
}

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
LCD5110_display lcd1;

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
		inline void udelay_asm (uint32_t useconds) {
		 useconds *= LOOP_FREQ;

		    asm volatile("   mov r0, %[useconds]    \n\t"
		                 "1: subs r0, #1            \n\t"
		                 "   bhi 1b                 \n\t"
		                 :
		                 : [useconds] "r" (useconds)
		                 : "r0");
		}
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

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
  MX_I2C1_Init();
  MX_SPI2_Init();
  MX_TIM10_Init();
  /* USER CODE BEGIN 2 */
  init_timing();
    lcd1.hw_conf.spi_handle = &hspi2;
  	lcd1.hw_conf.spi_cs_pin =  LCD1_CS_Pin;
  	lcd1.hw_conf.spi_cs_port = LCD1_CS_GPIO_Port;
  	lcd1.hw_conf.rst_pin =  LCD1_RST_Pin;
  	lcd1.hw_conf.rst_port = LCD1_RST_GPIO_Port;
  	lcd1.hw_conf.dc_pin =  LCD1_DC_Pin;
  	lcd1.hw_conf.dc_port = LCD1_DC_GPIO_Port;
  	lcd1.def_scr = lcd5110_def_scr;
  	LCD5110_init(&lcd1.hw_conf, LCD5110_NORMAL_MODE, 0x40, 2, 3);

      LCD5110_print("HEVALO DCP!\n", BLACK, &lcd1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
      HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);
  while (1)
  {
	  	  LCD5110_clear_scr(&lcd1);
	  	  LCD5110_set_cursor(0, 0, &lcd1);
	  	  //int e_state = HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin); // Could be 1 here!
	  	     int t_state = HAL_GPIO_ReadPin(TRIG_GPIO_Port, TRIG_Pin);
	  	     while(  t_state == GPIO_PIN_SET )
	  	     {
	  	    	LCD5110_clear_scr(&lcd1);
			  LCD5110_set_cursor(0, 0, &lcd1);
	  	    	 LCD5110_print("Wrong state before triggering, Trig is high\n", BLACK, &lcd1);
	  	      HAL_Delay(300);
	  	     }
	  	     HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_SET);
	  	     if ( HAL_GPIO_ReadPin(TRIG_GPIO_Port, TRIG_Pin) != GPIO_PIN_SET )
	  	     {
	  	    	LCD5110_clear_scr(&lcd1);
			  LCD5110_set_cursor(0, 0, &lcd1);
				 LCD5110_print("Line Trig do not went high while triggering.\n", BLACK, &lcd1);
	  	      HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);
	  	      HAL_Delay(300);
	  	      continue;
	  	     }
	  	     udelay(16);
	  	     HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);
	  	     if ( HAL_GPIO_ReadPin(TRIG_GPIO_Port, TRIG_Pin) != GPIO_PIN_RESET )
	  	     {
	  	    	LCD5110_clear_scr(&lcd1);
				  LCD5110_set_cursor(0, 0, &lcd1);
				 LCD5110_print("Line Trig do not went low after triggering.\n", BLACK, &lcd1);
	  	      HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);
	  	      HAL_Delay(300);
	  	      continue;
	  	     }
	  	     if ( HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == GPIO_PIN_SET )
	  	     {
	  	    	LCD5110_clear_scr(&lcd1);
				  LCD5110_set_cursor(0, 0, &lcd1);
				 LCD5110_print("Line ECHO is high too early.\n", BLACK, &lcd1);
	  	      HAL_Delay(300);
	  	      continue;
	  	     }

	  	     uint32_t watchdog_begin = get_us();
	  	     int didnt_had_1_at_echo = 0;
	  	     while(HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == GPIO_PIN_RESET )
	  	     {
	  	      if( get_us() - watchdog_begin > 500000 )
	  	      {
	  	       didnt_had_1_at_echo = 1;
	  	       break;
	  	      }
	  	     }
	  	     if(didnt_had_1_at_echo)
	  	     {
				  LCD5110_clear_scr(&lcd1);
				  LCD5110_set_cursor(0, 0, &lcd1);
				  LCD5110_print("Line ECHO didn't go high for a long time.\n", BLACK, &lcd1);
				  HAL_Delay(300);
				  continue;
	  	     }

	  	     uint32_t before = get_us();
	  	     int didnt_had_0_at_echo = 0;
	  	     while(HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == GPIO_PIN_SET )
	  	     {
	  	      if( get_us() - watchdog_begin > 500000 )
	  	      {
	  	    	  didnt_had_0_at_echo = 1;
	  	    	  break;
	  	      }
	  	     }
	  	     if(didnt_had_0_at_echo)
	  	     {
				  LCD5110_clear_scr(&lcd1);
				  LCD5110_set_cursor(0, 0, &lcd1);
				  LCD5110_print("Line ECHO didn't go low after echoing pulse stared for a long time.\n", BLACK, &lcd1);
				  HAL_Delay(300);
				  continue;
	  	     }


	  	     uint32_t pulse_time = get_us()-before;
	  	     uint32_t distance = pulse_time/58;
	  	     //! Увага, не забудьте додати:
	  	     // monitor arm semihosting enable
	  	     // До  Debug Configurations -> Startup Tab:
	  	     LCD5110_clear_scr(&lcd1);
	  	     LCD5110_set_cursor(0, 0, &lcd1);
	  	     LCD5110_printf(&lcd1, BLACK, "Time: %lu ms, distance: %lu cm\n", pulse_time, pulse_time*340/20);
	  	     HAL_Delay(500);

	  	     if( distance > 500 )
	  	     {
	  	    	LCD5110_clear_scr(&lcd1);
	  	    	LCD5110_set_cursor(0, 0, &lcd1);
	  	    	LCD5110_print("Too far -- possibly no echo at all.", BLACK, &lcd1);
	  	     }

//
//	HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_SET);
//	HAL_Delay(50);
//	HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);
//
//	while(HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == GPIO_PIN_RESET );
//	   {}
//
//	uint32_t before = HAL_GetTick();
//	while(HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == GPIO_PIN_SET );
//	{}
//	uint32_t pulse_time = HAL_GetTick()-before;
//	LCD5110_clear_scr(&lcd1);
//	LCD5110_set_cursor(0, 0, &lcd1);
//	LCD5110_printf(&lcd1, BLACK, "Time: %lu ms, distance: %lu cm\n", pulse_time, pulse_time*340/20);
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 8;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

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
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
