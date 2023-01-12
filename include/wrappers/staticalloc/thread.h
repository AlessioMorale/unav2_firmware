#ifndef THREAD_H
#define THREAD_H
#pragma once

#include <FreeRTOS.h>
#include <etl/delegate.h>
#include <stddef.h>
#include <string>
#include <wrappers/taskpriority.h>

namespace freertos::wrappers {

using thread_delegate = etl::delegate<void()>;

namespace staticalloc {
extern "C" {
inline static void BaseThreadWrapper(void *argument) {
  auto threadFunction = static_cast<freertos::wrappers::thread_delegate *>(const_cast<void *>(argument));
  (*threadFunction)();
}
}

template <size_t stackSize> class Thread {
public:
  Thread(const freertos::wrappers::thread_delegate &threadFunction, freertos::wrappers::TaskPriority priority, const std::string name)
      : Name{name}, Priority(priority), _threadFunction{threadFunction} {
  }

  void Create() {
    (void)xTaskCreateStatic(BaseThreadWrapper, Name.c_str(), stackSize, static_cast<void *>(&this->_threadFunction), static_cast<const UBaseType_t>(Priority),
                            _thread_stack, &_thread_taskdef);
  }

  virtual ~Thread() = default;
  const std::string &Name;
  const freertos::wrappers::TaskPriority Priority;

protected:
  freertos::wrappers::thread_delegate _threadFunction;
  StackType_t _thread_stack[stackSize];
  StaticTask_t _thread_taskdef;
};
} // namespace staticalloc
} // namespace freertos::wrappers
#endif /* THREAD_H */
