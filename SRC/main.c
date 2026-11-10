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
/* USER CODE BEGIN Header */
/**
  * @file           : main.c
  * @brief          : Main program body - Automatic Fish Feeder (Blue Pill)
  */
/* USER CODE END Header */

#include "main.h"
#include <stdio.h>
#include <string.h>

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MKE_M17_ADDR (0x40 << 1) // 7-bit I2C address 0x40 shifted left for STM32 HAL
/* USER CODE END PD */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
uint8_t rx_buffer[32];
uint8_t rx_index = 0;
char time_string[32];
uint8_t data_ready = 0;
uint32_t last_fed_tick = 0; // Cooldown tracking
const uint32_t COOLDOWN_MS = 600000; // 10 minutes rest
int current_hour = -1;
int current_minute = -1;

// Feeding Schedule (24-hour format)
typedef struct {
    int hour;
    int minute;
} FeedingTime;

FeedingTime schedule[] = {
    { 8, 0 },   // Morning feed: 08:00 AM
    { 18, 0 },  // Evening feed: 06:00 PM
    { 0, 8 },   // Test entries
    { 0, 5 },
    { 0, 10 },
    { 0, 14 },
    { 0, 18 },
    { 0, 22 },
    { 0, 26 },
    { 0, 30 },
    { 0, 34 },
};
int num_feedings = sizeof(schedule) / sizeof(schedule[0]);
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C1_Init(void);

/* USER CODE BEGIN PFP */
void execute_feed_cycle(void);
void motor_set_speed(uint8_t channel, int8_t speed);
int time_to_minutes(int hour, int minute);
uint8_t is_in_window(int current_total_min, int scheduled_hour, int scheduled_min);
/* USER CODE END PFP */

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_I2C1_Init();

  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart1, &rx_buffer[rx_index], 1);
  /* USER CODE END 2 */

  while (1)
  {
      if (data_ready) {
          data_ready = 0; // Clear UART flag

          // Parse time string received from ESP32 (handles both "HH:MM" or full timestamps)
          int parsed = 0;
          if (strchr(time_string, '-')) {
              // Full timestamp format: "YYYY-MM-DD HH:MM:SS" -> skip date
              int year, month, day, sec;
              parsed = sscanf(time_string, "%d-%d-%d %d:%d:%d", &year, &month, &day, &current_hour, &current_minute, &sec);
              if (parsed < 5) parsed = 0; else parsed = 2; // We got hour and minute
          } else {
              // Short format: "HH:MM"
              parsed = sscanf(time_string, "%d:%d", &current_hour, &current_minute);
          }

          if (parsed == 2) {
              int current_total_min = time_to_minutes(current_hour, current_minute);

              // Check if 10-minute cooldown since last feed has elapsed
              if ((HAL_GetTick() - last_fed_tick) >= COOLDOWN_MS || last_fed_tick == 0) {

                  for (int i = 0; i < num_feedings; i++) {
                      if (is_in_window(current_total_min, schedule[i].hour, schedule[i].minute)) {

                          // Execute feed cycle
                          execute_feed_cycle();

                          // Record time tick to activate 10-minute cooldown
                          last_fed_tick = HAL_GetTick();
                          break; // Exit schedule loop
                      }
                  }
              }
          }
      }
  }
}

/**
  * @brief System Clock Configuration
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  // Configure Hall Sensor Pin as Input with Pull-Up (PB1)
  GPIO_InitStruct.Pin = HALL_SENSOR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(HALL_SENSOR_GPIO_Port, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */
int time_to_minutes(int hour, int minute) {
    return (hour * 60) + minute;
}

uint8_t is_in_window(int current_total_min, int scheduled_hour, int scheduled_min) {
    int scheduled_total_min = time_to_minutes(scheduled_hour, scheduled_min);
    int diff = current_total_min - scheduled_total_min;
    return (diff >= -10 && diff <= 10);
}

// Helper to send I2C speed command to MKE-M17 motor driver using defined macro
void motor_set_speed(uint8_t channel, int8_t speed) {
    uint8_t data[2] = {channel, (uint8_t)speed};
    HAL_I2C_Master_Transmit(&hi2c1, MKE_M17_ADDR, data, 2, 100);
}

void execute_feed_cycle(void) {
    // 1. Turn ON motor via I2C (Channel 1, Speed = 150)
    motor_set_speed(1, 150);

    uint32_t start_tick = HAL_GetTick();
    uint32_t timeout_ms = 10000; // 10-second safety timeout against jams

    // 2. Clearance phase: If starting directly on a magnet (LOW), wait until it leaves (HIGH)
    if (HAL_GPIO_ReadPin(HALL_SENSOR_GPIO_Port, HALL_SENSOR_Pin) == GPIO_PIN_RESET) {
        while (HAL_GPIO_ReadPin(HALL_SENSOR_GPIO_Port, HALL_SENSOR_Pin) == GPIO_PIN_RESET) {
            if ((HAL_GetTick() - start_tick) > timeout_ms) break;
            HAL_Delay(5);
        }
    }

    // 3. Rotation phase: Wait until the magnet sweeps past and triggers the sensor again (LOW)
    while (HAL_GPIO_ReadPin(HALL_SENSOR_GPIO_Port, HALL_SENSOR_Pin) == GPIO_PIN_SET) {
        if ((HAL_GetTick() - start_tick) > timeout_ms) {
            break; // Stop motor if jammed
        }
        HAL_Delay(5);
    }

    // 4. Turn OFF motor via I2C immediately (Speed = 0)
    motor_set_speed(1, 0);
}

// UART Interrupt Callback to capture streaming time data
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        if (rx_buffer[rx_index] == '\n' || rx_index >= sizeof(time_string) - 1) {
            time_string[rx_index] = '\0'; // Null-terminate string
            rx_index = 0;
            data_ready = 1;
        } else {
            rx_index++;
        }
        HAL_UART_Receive_IT(&huart1, &rx_buffer[rx_index], 1);
    }
}
/* USER CODE END 4 */

void Error_Handler(void)
{
  __disable_irq();
  while (1) {}
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
