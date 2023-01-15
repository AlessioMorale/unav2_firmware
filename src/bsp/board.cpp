#include <bsp/board.h>
#include <stm32f4xx.h>
#include <stm32f4xx_hal.h>
#include <tusb.h>

namespace unav::bsp {
Board Board::instance;
void Board::init_gpio() {
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
}
void Board::init_usb_hw() {
  init_usb_pins();
  board_vbus_sense_init();
}

void Board::init_leds(){

  for(auto gpio : leds)
  {
    gpio.init();
  }
}

void Board::board_vbus_sense_init() {
  USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_NOVBUSSENS;
  USB_OTG_FS->GCCFG &= ~USB_OTG_GCCFG_VBUSBSEN;
  USB_OTG_FS->GCCFG &= ~USB_OTG_GCCFG_VBUSASEN;
}

void Board::init_usb_pins() {

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

} // namespace unav::bsp

//--------------------------------------------------------------------+
// Forward USB interrupt events to TinyUSB IRQ Handler
//--------------------------------------------------------------------+
extern "C" {

void OTG_FS_IRQHandler(void) {
  tud_int_handler(BOARD_TUD_RHPORT);
}
}
