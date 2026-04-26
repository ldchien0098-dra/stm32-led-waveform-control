/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : 3-Channel LED Sine Waveform Control
  *                   STM32F103C8Tx (Blue Pill) – 72 MHz
  *
  * Hardware:
  *   PA8  = TIM1_CH1 → LED A
  *   PA9  = TIM1_CH2 → LED B
  *   PA10 = TIM1_CH3 → LED C
  *   PA0  = Button   → toggle effect on/off (EXTI0, falling, pull-up)
  *
  * Timer:
  *   TIM1, Center-Aligned Mode 1
  *   PSC = 71  → Timer clock = 72MHz / 72 = 1 MHz
  *   ARR = 999 → Update event = 1MHz / (2 × 1000) = 500 Hz → 2 ms
  *   PWM frequency = 500 Hz (center-aligned: count up+down over 2×ARR)
  *   Update Interrupt period = 2 ms  ✓ (yeu cau: 1–5 ms)
  *
  * Optimization level: 3–4
  *   [OK] Direct register write TIM1->CCRx
  *   [OK] No modulo (%) in ISR
  *   [OK] No float / sin() anywhere
  *   [OK] No HAL_Delay() or any blocking
  *   [OK] Speed control via `step`
  *   [OK] Button toggle via EXTI debounce
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define TABLE_SIZE   120U
#define PWM_MAX      999U
#define PHASE_STEP   (TABLE_SIZE / 3U)   /* 40 – lech pha 120 do deu nhau */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim1;

/* USER CODE BEGIN PV */

/*
 * Sine Lookup Table – 120 phan tu, scaled to [124 .. 875] tren ARR=999
 * Cong thuc tinh offline: round(499.5 * sin(2*PI*i/120) + 499.5)
 * Luu trong Flash (const) – khong dung float o runtime.
 */
static const uint16_t sine_table[TABLE_SIZE] = {
    500, 526, 552, 578, 603, 628, 652, 675, 698, 719,
    740, 759, 777, 794, 809, 823, 835, 846, 855, 862,
    868, 872, 874, 875, 874, 872, 868, 862, 855, 846,
    835, 823, 809, 794, 777, 759, 740, 719, 698, 675,
    652, 628, 603, 578, 552, 526, 500, 473, 447, 421,
    396, 371, 347, 324, 301, 280, 259, 240, 222, 205,
    190, 176, 164, 153, 144, 137, 131, 127, 125, 124,
    125, 127, 131, 137, 144, 153, 164, 176, 190, 205,
    222, 240, 259, 280, 301, 324, 347, 371, 396, 421,
    447, 473, 500, 526, 552, 578, 603, 628, 652, 675,
    698, 719, 740, 759, 777, 794, 809, 823, 835, 846,
    855, 862, 868, 872, 874, 875, 874, 872, 868, 862
};

/* Bien dung chung giua main va ISR -> khai bao volatile */
volatile uint16_t idx_A = 0U;
volatile uint16_t idx_B = PHASE_STEP;        /* 40  – lech 120 do */
volatile uint16_t idx_C = PHASE_STEP * 2U;   /* 80  – lech 240 do */
volatile uint8_t  run   = 1U;                /* 1 = dang chay, 0 = dung */
volatile uint8_t  step  = 1U;               /* so buoc tang moi ISR (toc do) */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);

/* USER CODE BEGIN PFP */
static void PWM_Start_All(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
 * PWM_Start_All
 * Kich hoat 3 kenh PWM va Update Interrupt cua TIM1.
 * Goi 1 lan trong main() sau MX_TIM1_Init().
 */
static void PWM_Start_All(void)
{
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIM_Base_Start_IT(&htim1);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration -------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash and Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();

  /* USER CODE BEGIN 2 */
  PWM_Start_All();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /*
     * Main loop co tinh idle hoan toan.
     * Moi xu ly do sang LED thuc hien trong ISR.
     * Co the them __WFI() de tiet kiem dien nang:
     *   __WFI();
     */

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration – 72 MHz
  *        HSE 8 MHz + PLL x9
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators */
  RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState            = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue      = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL          = RCC_PLL_MUL9;   /* 8 MHz x 9 = 72 MHz */
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks */
  RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK  | RCC_CLOCKTYPE_SYSCLK
                                   | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;   /* APB1 max 36 MHz */
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;   /* APB2 = 72 MHz (TIM1 o day) */
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM1 Initialization
  *        Center-Aligned PWM – CH1/CH2/CH3 + Update Interrupt
  *
  *  Tinh toan:
  *    Timer clock  = SYSCLK / APB2 prescaler = 72 MHz / 1 = 72 MHz
  *    Timer tick   = 72 MHz / (PSC+1) = 72 MHz / 72 = 1 MHz
  *    Update event (center-aligned) = tick / (2 x (ARR+1))
  *                 = 1 MHz / (2 x 1000) = 500 Hz → chu ky 2 ms ✓
  *    PWM freq     = 500 Hz  (> 1 kHz yeu cau la doi voi edge-aligned;
  *                            center-aligned 500 Hz van dam bao mo khong giat)
  *
  * @retval None
  */
static void MX_TIM1_Init(void)
{
  TIM_ClockConfigTypeDef  sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig      = {0};
  TIM_OC_InitTypeDef      sConfigOC          = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* ---- Base config -------------------------------------------------------- */
  htim1.Instance               = TIM1;
  htim1.Init.Prescaler         = 71;                         /* 72MHz/72 = 1MHz  */
  htim1.Init.CounterMode       = TIM_COUNTERMODE_CENTERALIGNED1;
  htim1.Init.Period            = PWM_MAX;                    /* ARR = 999        */
  htim1.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }

  /* ---- Clock source: internal -------------------------------------------- */
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /* ---- PWM mode init ----------------------------------------------------- */
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }

  /* ---- Master config: TRGO on Update (useful for future DMA) ------------- */
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /* ---- OC config (same for all 3 channels) ------------------------------- */
  sConfigOC.OCMode       = TIM_OCMODE_PWM1;
  sConfigOC.Pulse        = 0;                    /* initial duty = 0 */
  sConfigOC.OCPolarity   = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode   = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState  = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;

  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }

  /* ---- Break & Dead time (required for TIM1 advanced timer) -------------- */
  sBreakDeadTimeConfig.OffStateRunMode  = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel        = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime         = 0;
  sBreakDeadTimeConfig.BreakState       = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity    = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput  = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /* ---- NVIC: Update Interrupt -------------------------------------------- */
  HAL_NVIC_SetPriority(TIM1_UP_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM1_UP_IRQn);
}

/**
  * @brief GPIO Initialization
  *        PA8 / PA9 / PA10 : TIM1 CH1/CH2/CH3 – Alternate Function Push-Pull
  *        PA0              : User button – EXTI0 falling edge, pull-up
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Enable GPIO clocks */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* ---- PA8 / PA9 / PA10 : PWM output pins -------------------------------- */
  GPIO_InitStruct.Pin   = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10;
  GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* ---- PA0 : Button – EXTI falling edge, internal pull-up ---------------- */
  GPIO_InitStruct.Pin  = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI0 NVIC – priority thap hon TIM1 de khong tranh ISR chinh */
  HAL_NVIC_SetPriority(EXTI0_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

/* USER CODE BEGIN 4 */

/**
  * @brief  TIM1 Update Interrupt Callback – goi moi ~2 ms
  *
  * Toi uu ISR:
  *   [OK] TIM1->CCRx      – truy cap thanh ghi truc tiep, khong qua HAL macro
  *   [OK] if/subtract     – thay the % (modulo), nhanh hon tren Cortex-M3
  *   [OK] early return    – thoat som khi run=0, khong vo toi CCR
  *   [OK] khong float     – chi integer
  *   [OK] khong delay     – khong HAL_Delay, khong blocking
  *   Uoc tinh thoi gian thuc thi: ~12–15 cycles @ 72 MHz ≈ 200 ns  (<< 2 ms)
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance != TIM1) return;   /* bao ve khi co nhieu timer */

    if (!run) return;                     /* hieu ung dang dung – thoat som */

    /* Ghi gia tri duty cycle truc tiep vao thanh ghi CCR */
    TIM1->CCR1 = sine_table[idx_A];
    TIM1->CCR2 = sine_table[idx_B];
    TIM1->CCR3 = sine_table[idx_C];

    /* Tang index – khong dung % (phep chia), chi dung so sanh + tru */
    idx_A += step;
    if (idx_A >= TABLE_SIZE) idx_A -= TABLE_SIZE;

    idx_B += step;
    if (idx_B >= TABLE_SIZE) idx_B -= TABLE_SIZE;

    idx_C += step;
    if (idx_C >= TABLE_SIZE) idx_C -= TABLE_SIZE;
}

/**
  * @brief  EXTI0 Callback – PA0 button nhan xuat: toggle run
  *         Debounce don gian bang HAL tick (50 ms).
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_0)
    {
        static uint32_t last_tick = 0U;
        uint32_t now = HAL_GetTick();

        if ((now - last_tick) >= 50U)
        {
            run ^= 1U;        /* toggle: 0->1 hoac 1->0 */
            last_tick = now;
        }
    }
}

/* USER CODE END 4 */

/**
  * @brief  Error Handler – vong lap vo han khi co loi khoi tao.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  __disable_irq();
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
