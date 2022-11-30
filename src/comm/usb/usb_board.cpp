#include "FreeRTOS.h"
#include <stm32f4xx.h>
#include <stm32f4xx_hal.h>

static void init_usb_pins(void);
static void board_vbus_sense_init(void);

void init_usb_hw() {
  board_vbus_sense_init();
  init_usb_pins();
}

static void board_vbus_sense_init(void) {
  // Blackpill doens't use VBUS sense (B device) explicitly disable it
  USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_NOVBUSSENS;
  USB_OTG_FS->GCCFG &= ~USB_OTG_GCCFG_VBUSBSEN;
  USB_OTG_FS->GCCFG &= ~USB_OTG_GCCFG_VBUSASEN;
}

static void init_usb_pins(void) {

  // Enable USB OTG clock
  __HAL_RCC_USB_OTG_FS_CLK_ENABLE();
  NVIC_SetPriority(OTG_FS_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);

  /* Configure USB FS GPIOs */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  /* Configure USB D+ D- Pins */
  GPIO_InitTypeDef gpio_out = {
      .Pin = GPIO_PIN_11 | GPIO_PIN_12, .Mode = GPIO_MODE_AF_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_VERY_HIGH, .Alternate = GPIO_AF10_OTG_FS};

  HAL_GPIO_Init(GPIOA, &gpio_out);
}
