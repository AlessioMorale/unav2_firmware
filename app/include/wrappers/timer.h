#ifndef WRAPPER_TIMER_H
#define WRAPPER_TIMER_H
#pragma once

#include <FreeRTOS.h>
#include <etl/delegate.h>
#include <stddef.h>
#include <timers.h>

#include <string>

#include "private/wrapper_utils.h"
namespace freertos::wrappers {

using void_delegate = etl::delegate<void()>;

extern "C" {
inline static void BaseTimerWrapper(TimerHandle_t timer_handle) {
  auto timer_id = pvTimerGetTimerID(timer_handle);
  auto timer_function = static_cast<freertos::wrappers::void_delegate *>(const_cast<void *>(timer_id));
  (*timer_function)();
}

class Timer {
  const TickType_t DEFAULT_BLOCKTIME = 10;

 public:
  Timer(const freertos::wrappers::void_delegate &timer_function, const std::string &name, bool auto_reload,
        uint32_t timer_period_us)
      : Name{name}, timer_function_{timer_function} {
    auto ticks = utils::us_to_freertos_ticks(timer_period_us);
    UBaseType_t reload = auto_reload ? pdTRUE : pdFALSE;
    timer_handler_ = xTimerCreateStatic(Name.c_str(), ticks, reload, static_cast<void *>(&timer_function_),
                                        BaseTimerWrapper, &timer_buffer_);
  }

  void start() { xTimerStart(timer_handler_, DEFAULT_BLOCKTIME); }
  void stop() { xTimerStop(timer_handler_, DEFAULT_BLOCKTIME); }

  void start_from_isr() {
    auto higher_priority_task_woken = pdFALSE;
    xTimerStartFromISR(timer_handler_, &higher_priority_task_woken);
    portYIELD_FROM_ISR(higher_priority_task_woken);
  }
  void stop_from_isr() {
    auto higher_priority_task_woken = pdFALSE;
    xTimerStopFromISR(timer_handler_, &higher_priority_task_woken);
    portYIELD_FROM_ISR(higher_priority_task_woken);
  }

  void reset() { xTimerReset(timer_handler_, DEFAULT_BLOCKTIME); }
  void reset_from_isr() {
    auto higher_priority_task_woken = pdFALSE;

    xTimerResetFromISR(timer_handler_, &higher_priority_task_woken);
    portYIELD_FROM_ISR(higher_priority_task_woken);
  }

  void set_period(uint32_t new_period_us) {
    auto ticks = utils::us_to_freertos_ticks(new_period_us);
    xTimerChangePeriod(timer_handler_, ticks, DEFAULT_BLOCKTIME);
  }

  void set_period_from_isr(uint32_t new_period_us) {
    auto ticks = utils::us_to_freertos_ticks(new_period_us);
    auto higher_priority_task_woken = pdFALSE;
    xTimerChangePeriodFromISR(timer_handler_, ticks, &higher_priority_task_woken); 
    portYIELD_FROM_ISR(higher_priority_task_woken);
  }

  void set_reload_mode(bool auto_reload) {
    UBaseType_t reload = auto_reload ? pdTRUE : pdFALSE;
    vTimerSetReloadMode(timer_handler_, reload);
  }

  virtual ~Timer() = default;
  const std::string Name;

 protected:
  freertos::wrappers::void_delegate timer_function_;
  StaticTimer_t timer_buffer_;
  TimerHandle_t timer_handler_;
};
}
}  // namespace freertos::wrappers
#endif /* WRAPPER_TIMER_H */
