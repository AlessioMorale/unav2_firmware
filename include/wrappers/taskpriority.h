#ifndef TASKPRIORITY_H
#define TASKPRIORITY_H
#pragma once
#include "FreeRTOS.h"

namespace freertos::wrappers {
enum class TaskPriority : UBaseType_t {
  Idle = 0,                                                         ///< Non-Real Time operations. tasks that don't block
  Low = ((configMAX_PRIORITIES) > 1),                               ///< Non-Critical operations
  HMI = (Low + ((configMAX_PRIORITIES) > 5)),              ///< Normal User Interface Level
  Mid = ((configMAX_PRIORITIES) / 2),                               ///< Semi-Critical, have deadlines, not a lot of processing
  High = ((configMAX_PRIORITIES)-1 - ((configMAX_PRIORITIES) > 4)), ///< Urgent tasks, short deadlines, not much processing
  Highest = ((configMAX_PRIORITIES)-1)
};
}
#endif /* TASKPRIORITY_H */
