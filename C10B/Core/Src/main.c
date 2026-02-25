/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
float ranger_dis=0;
float temperature=0;
uint8_t k210_buffer[30];
uint8_t k210_active_cnt = 0;
uint8_t k210_active_flag=0;

C10B_Sendmsg c10b_send = { 0 };
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

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
  MX_DMA_Init();
  MX_UART5_Init();
  MX_USART3_UART_Init();
  MX_TIM2_Init();
  MX_TIM7_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	DWT_Init();
	OLED_Init();
	
	//启动超声波通道
	HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_2);
	
	//MPU6050初始化
	MPU6050_initialize();
	DMP_Init();
	
	//启动DMA接收K210数据
	HAL_UARTEx_ReceiveToIdle_DMA(&huart3,k210_buffer,30);
			
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  //姿态显示
	  OLED_ShowString(0,0,"Pitch");
	  OLED_ShowString(0,10,"Roll");
	  OLED_ShowString(0,20,"Yaw");
	  OLED_ShowFloat(50,0,Pitch,3,2);
	  OLED_ShowFloat(50,10,Roll,3,2);
	  OLED_ShowFloat(50,20,Yaw,3,2);
	  
	  //温度显示
	  OLED_ShowString(0,30,"temp");
	  OLED_ShowFloat(50,30,temperature,3,2);
	  
	  //颜色显示
	  OLED_ShowString(0,40,"color");
		if(k210.color==0){
			OLED_ShowString(50,40,"red");
		}else if(k210.color==1){
			OLED_ShowString(50,40,"green");
		}else if(k210.color==2){
			OLED_ShowString(50,40,"yellow");
		}
	  
	  //巡线中心与识别面积显示
	  OLED_ShowNumber(30,50,k210.cx,3,12);
	  OLED_ShowNumber(60,50,k210.size,4,12);

	  OLED_Refresh_Gram();
	  delay_ms(10);
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
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
//串口重定向
UART_HandleTypeDef *DebugSerial = &huart1;

//printf函数实现
int fputc(int ch,FILE* stream)
{
	while( HAL_OK != HAL_UART_Transmit(DebugSerial,(const uint8_t *)&ch,1,100));
	return ch;
}

//MPU6050外部中断
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	static uint8_t ranger_cnt = 0;
	static uint32_t temperature_cnt = 0;
	static uint8_t c50c_send_cnt = 0;
	
	uint8_t tempbuffer[2];
	
	//MPU6050 外部中断引脚触发 频率为 DEFAULT_MPU_HZ
	if( GPIO_Pin == MPU6050_INT_Pin )
	{
		//获取温度值
		temperature_cnt++;
		if( temperature_cnt>=100*5 )
		{
			temperature_cnt=0;
			pIICInterface_t iic = &User_sIICDev;
			iic->read_reg(0x68<<1,0x41,tempbuffer,2,200);
			temperature = (float)((short)(tempbuffer[0]<<8|tempbuffer[1]))/340.0f+36.53f;//温度数据
		}
		
		//陀螺仪数据读取
		 Read_DMP();
		
		//触发超声波工作 20Hz
		ranger_cnt++;
		if( ranger_cnt>=5 )
		{
			ranger_cnt=0;
			HAL_GPIO_WritePin(ranger_tri_GPIO_Port,ranger_tri_Pin,GPIO_PIN_RESET);
			delay_us(15);
			HAL_GPIO_WritePin(ranger_tri_GPIO_Port,ranger_tri_Pin,GPIO_PIN_SET);
		}
		
		//k210活跃检测
		k210_active_cnt++;
		if( k210_active_cnt>=100*1 )
		{
			k210_active_flag = 0;
		}
		
		//20Hz频率传输数据到C50C
		c50c_send_cnt++;
		if( c50c_send_cnt>=5 )
		{
			c50c_send_cnt = 0;
			c10b_send.Head1 = C10B_HEAD1;
			c10b_send.Head2 = C10B_HEAD2;
			c10b_send.k210_alive = k210_active_flag;
			c10b_send.k210_cx = k210.cx;
			c10b_send.k210_size = k210.size;
			c10b_send.color = k210.color;
			c10b_send.temperature = temperature*100;
			uint8_t* sendptr = (uint8_t*)&c10b_send;
			c10b_send.BCCcheck = calculateBCC(sendptr,sizeof(c10b_send)-1);
			HAL_UART_Transmit(&huart5,sendptr,sizeof(c10b_send),500);
		}
	}
}

//超声波采集辅助变量
uint32_t upvalue=0;
uint32_t downvalue=0;
//超声波采集中断
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	#define TIM_COUNT_FREQ 1000000.0f
	enum {
		WaitUp=0,
		WaitDown,
	};
	static uint8_t sm = WaitUp;
	
	//超声波接口触发中断
	if( htim == &htim2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2 )
	{
		switch (sm)
		{
			case WaitUp:
			{
				//读出计数值
				upvalue = HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_2);
				
				//设置下降沿捕获
				TIM_RESET_CAPTUREPOLARITY(&htim2,TIM_CHANNEL_2);
				TIM_SET_CAPTUREPOLARITY(&htim2,TIM_CHANNEL_2,TIM_ICPOLARITY_FALLING);
				
				//状态机切换
				sm = WaitDown;
				break;
			}
			
			case WaitDown:
			{
				//读取计数值
				downvalue = HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_2);
				
				//计算距离
				if( upvalue > downvalue )//溢出计算
					ranger_dis = downvalue+0xFFFF - upvalue;
				else
					ranger_dis = downvalue - upvalue;
				
				// S = V*t/2
				ranger_dis = (ranger_dis/TIM_COUNT_FREQ)/2.0f * 340.0f;
				
				//设置上升沿捕获
				TIM_RESET_CAPTUREPOLARITY(&htim2,TIM_CHANNEL_2);
				TIM_SET_CAPTUREPOLARITY(&htim2,TIM_CHANNEL_2,TIM_ICPOLARITY_RISING);
				
				//状态机切换
				sm = WaitUp;
				
				break;
			}
		}	
	}

}

//串口DMA中断,处理K210数据
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if( huart == &huart3 && HAL_UART_STATE_READY == huart->RxState ) 
	{
		//活跃状态刷新
		k210_active_cnt=0;
		k210_active_flag=1;
		
		//解析k210数据
		for(uint8_t i=0;i<Size;i++)
		{
			k210_data_callback(k210_buffer[i]);
		}
		
		//重新使能中断
		HAL_UARTEx_ReceiveToIdle_DMA(&huart3,k210_buffer,30);
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
