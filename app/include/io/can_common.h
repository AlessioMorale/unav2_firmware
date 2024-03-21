// add a standard c header guard
#pragma once
#ifndef CAN_COMMON_H
#define CAN_COMMON_H

#include <cstdint>

struct canfd_frame {
uint32_t echo_id;
uint32_t can_id;
uint8_t can_dlc;
uint8_t channel;
uint8_t flags;
uint8_t reserved;
uint8_t data[64];
uint32_t timestamp_us;
} __packed __aligned(4);

#endif // CAN_COMMON_H