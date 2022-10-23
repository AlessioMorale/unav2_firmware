#include <stm32f4xx_hal.h>
#include "pb.h"
#include "etl/vector.h"

extern "C" int main()
{
  // Switch on blue LED on STM32F407Discovery

  __HAL_RCC_GPIOD_CLK_ENABLE();

  GPIO_InitTypeDef gpio_out = {
    .Pin = GPIO_PIN_15,
    .Mode = GPIO_MODE_OUTPUT_PP,
    .Pull = GPIO_NOPULL,
    .Speed = GPIO_SPEED_FREQ_HIGH
  };

  HAL_GPIO_Init(GPIOD, &gpio_out);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);

  for (;;);

  return 0;
}
