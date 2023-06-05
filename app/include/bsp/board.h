#pragma once
#ifndef BOARD_BSP_H
#define BOARD_BSP_H
#include <etl/array.h>
#include <stm32f4xx.h>
#include <stm32f4xx_hal.h>

#include <cstdint>
namespace unav::bsp {

struct gpio {
  uint32_t pin;
  uint32_t mode;
  uint32_t pull;
  uint32_t speed;
  GPIO_TypeDef *port;
  bool invert;

  bool is_valid() { return port != nullptr; }

  void init() {
    if (is_valid()) {
      GPIO_InitTypeDef gpio_out = {.Pin = pin, .Mode = mode, .Pull = pull, .Speed = speed};
      HAL_GPIO_Init(port, &gpio_out);
    }
  }

  void set(bool state) {
    if (is_valid()) {
      HAL_GPIO_WritePin(port, pin, state != invert ? GPIO_PinState::GPIO_PIN_SET : GPIO_PinState::GPIO_PIN_RESET);
    }
  }

  void toggle() {
    if (is_valid()) {
      HAL_GPIO_TogglePin(port, pin);
    }
  }
};

enum class BoardLeds { activity = 0, warning = 1, error = 2, count };

class Board {
 public:
  static Board instance;
  void init_usb_hw();
  void init_gpio();

  void init_leds();
  const etl::array<gpio, static_cast<size_t>(BoardLeds::count)> leds = {
      {GPIO_PIN_13, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIOC, false}};
  const gpio &led(BoardLeds led) { return leds[static_cast<size_t>(led)]; }

  const uint32_t[3] get_uid() {
    uint32_t[3] id = {HAL_GetUIDw0(), HAL_GetUIDw1(), HAL_GetUIDw2()};
    return id;
  }

 private:
  void board_vbus_sense_init();
  void init_usb_pins();
};
}  // namespace unav::bsp

#endif /* BOARD_BSP_H */
