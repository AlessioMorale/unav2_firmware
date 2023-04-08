#ifndef WRAPPER_UTILS_H
#define WRAPPER_UTILS_H
#pragma once
#include <FreeRTOS.h>
namespace freertos::wrappers::utils {

inline static TickType_t us_to_freertos_ticks(uint32_t time_us){
    auto ticks = static_cast<TickType_t>((time_us / 1000) / portTICK_PERIOD_MS);
    return ticks;
}
}
#endif //  WRAPPER_UTILS_H