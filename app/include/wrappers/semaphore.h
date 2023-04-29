#include <FreeRTOS.h>
#include <semphr.h>
#include <timing.h>

namespace freertos::wrappers {

class Semaphore {
public:
  Semaphore() {
    _semaphore = xSemaphoreCreateBinaryStatic(&_seemaphore_buf);
  }

  bool available() {
    return uxSemaphoreGetCount(_semaphore) > 0;
  }

  bool take(uint32_t wait_us) {
    auto ticks = unav::Timing::us_to_ticks(wait_us);
    auto r = xSemaphoreTake(_semaphore, ticks);
    return r == pdTRUE;
  }

  bool take_from_isr() {
    auto higher_priority_task_woken = pdFALSE;
    auto r = xSemaphoreTakeFromISR(_semaphore, &higher_priority_task_woken);
    portYIELD_FROM_ISR(higher_priority_task_woken);
    return r == pdTRUE;
  }

  bool give() {
    auto r = xSemaphoreGive(_semaphore);
    return r == pdTRUE;
  }

  bool give_from_isr() {
    auto higher_priority_task_woken = pdFALSE;
    auto r = xSemaphoreGiveFromISR(_semaphore, &higher_priority_task_woken);
    portYIELD_FROM_ISR(higher_priority_task_woken);
    return r == pdTRUE;
  }

private:
  SemaphoreHandle_t _semaphore;
  StaticSemaphore_t _seemaphore_buf;
};

} // namespace freertos::wrappers