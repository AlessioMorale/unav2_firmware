#pragma once
#ifndef BOARD_BSP_H
#define BOARD_BSP_H
#include <stm32f4xx.h>
#include <stm32f4xx_hal.h>
#include <tusb.h>

namespace unav::comm::usb {

namespace _private {
inline static void board_vbus_sense_init() {
  // Blackpill doens't use VBUS sense (B device) explicitly disable it
  USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_NOVBUSSENS;
  USB_OTG_FS->GCCFG &= ~USB_OTG_GCCFG_VBUSBSEN;
  USB_OTG_FS->GCCFG &= ~USB_OTG_GCCFG_VBUSASEN;
}

inline static void init_usb_pins() {

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
}

void init_usb_hw() {
  _private::board_vbus_sense_init();
  _private::init_usb_pins();
}

} // namespace unav::comm::usb

//--------------------------------------------------------------------+
// Forward USB interrupt events to TinyUSB IRQ Handler
//--------------------------------------------------------------------+
extern "C" {

void OTG_FS_IRQHandler(void) {
  tud_int_handler(0);
}

}
#endif /* BOARD_BSP_H */
