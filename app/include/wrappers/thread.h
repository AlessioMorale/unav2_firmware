#ifndef THREAD_H
#define THREAD_H
#pragma once

#include <FreeRTOS.h>
#include <etl/delegate.h>
#include <stddef.h>
#include <string>

namespace freertos::wrappers {

using thread_delegate = etl::delegate<void()>;

extern "C" {
inline static void BaseThreadWrapper(void *argument) {
  auto threadFunction = static_cast<freertos::wrappers::thread_delegate *>(const_cast<void *>(argument));
  (*threadFunction)();
}
}

enum class ThreadPriority : UBaseType_t {
  Idle = 0,                                   ///< Non-Real Time operations. tasks that don't block
  Low = ((configMAX_PRIORITIES) > 1),         ///< Non-Critical operations
  HMI = (Low + ((configMAX_PRIORITIES) > 5)), ///< Normal User Interface Level
  Mid = ((configMAX_PRIORITIES) / 2),         ///< Semi-Critical, have deadlines, not a lot of processing
  High =
      ((configMAX_PRIORITIES)-1 - ((configMAX_PRIORITIES) > 4)), ///< Urgent tasks, short deadlines, not much processing
  Highest = ((configMAX_PRIORITIES)-1)
};

template <size_t stack_size> class Thread {

public:
  Thread(const freertos::wrappers::thread_delegate &threadFunction, freertos::wrappers::ThreadPriority priority,
         const std::string &name)
      : Name{name}, Priority(priority), _threadFunction{threadFunction} {
  }

  void create() {
    (void)xTaskCreateStatic(BaseThreadWrapper, Name.c_str(), stack_size, static_cast<void *>(&this->_threadFunction),
                            static_cast<const UBaseType_t>(Priority), _thread_stack, &_thread_taskdef);
  }

  virtual ~Thread() = default;
  const std::string Name;
  const freertos::wrappers::ThreadPriority Priority;

protected:
  freertos::wrappers::thread_delegate _threadFunction;
  StackType_t _thread_stack[stack_size];
  StaticTask_t _thread_taskdef;
};
} // namespace freertos::wrappers
#endif /* THREAD_H */
