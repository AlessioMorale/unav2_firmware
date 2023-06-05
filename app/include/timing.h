#ifndef TIMING_H
#define TIMING_H
#pragma once
#include <etl/limits.h>
#include "stm32f4xx_hal.h"
namespace unav {

typedef uint32_t tickstime_t;

class Timing {
public:
  static const uint32_t MAX_DELAY = etl::numeric_limits<uint32_t>::max();

  static void init() {
    uint32_t hclk = HAL_RCC_GetHCLKFreq();
    ticks_per_us = hclk / 1000000;
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  }

  inline static tickstime_t get_ticks() {
    return DWT->CYCCNT;
  }

  inline static uint32_t get_us_since(const tickstime_t raw_start) {
    const tickstime_t raw_now = get_ticks();
    return (raw_now - raw_start) / ticks_per_us;
  }

  inline static uint32_t get_us() {
    return get_ticks() / ticks_per_us;
  }

  inline static uint32_t get_ms() {
    return get_us() / 1000ul;
  }

  inline static tickstime_t us_to_ticks(uint32_t us) {
    if (us == MAX_DELAY) {
      return portMAX_DELAY;
    }
    return static_cast<tickstime_t>(us * ticks_per_us);
  }

  inline static tickstime_t ms_to_ticks(uint32_t ms) {
    if (ms == MAX_DELAY) {
      return portMAX_DELAY;
    }
    return us_to_ticks(ms * 1000ul);
  }

  inline static uint32_t ticks_to_us(tickstime_t ticks) {
    if (ticks == portMAX_DELAY) {
      return MAX_DELAY;
    }
    return ticks / ticks_per_us;
  }

  inline static uint32_t ticks_to_ms(tickstime_t ticks) {
    if (ticks == portMAX_DELAY) {
      return MAX_DELAY;
    }
    return ticks_to_us(ticks) / 1000ul;
  }

private:
  Timing() = delete;
  inline static uint32_t ticks_per_us;
};
} // namespace unav

#endif /* TIMING_H */
