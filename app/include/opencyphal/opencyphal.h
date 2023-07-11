#pragma once
#include "timing.h"
#ifndef OPENCYPHAL_H
#define OPENCYPHAL_H
#include <FreeRTOS.h>
#include <debug.h>
#include <io/usb/gs_can/gs_usb.h>
#include <uavcan/_register/Access_1_0.h>
#include <uavcan/_register/List_1_0.h>
#include <uavcan/node/ExecuteCommand_1_1.h>
#include <uavcan/node/GetInfo_1_0.h>
#include <uavcan/node/port/List_0_1.h>
#include <wrappers/timer.h>

#include <cstring>
#include <memory>

#include "canard_wrapper.h"

#define KILO 1000L
#define MEGA ((int64_t)KILO * KILO)
namespace unav::opencyphal {

#define OC_STACK_SIZE configMINIMAL_STACK_SIZE + 512
class OpenCyphal {
 public:
  void setup(IQueue<unav::io::usb::UsbCan::gs_host_frame>& tx_queue,
             IQueue<unav::io::usb::UsbCan::gs_host_frame>& rx_queue) {
    logger_debug("OpenCyphal.Setup()!\r\n");
    canard_.setup_canard();
    tx_can_queue_ = &tx_queue;
    rx_can_queue_ = &rx_queue;

    can_task.create();
  };

  OpenCyphal()
      : can_task{thread_delegate::create<OpenCyphal, &OpenCyphal::can_comm_task_function>(*this), ThreadPriority::High,
                 "cyph/com"} {}

 private:
  Canard canard_ = {};

  IQueue<unav::io::usb::UsbCan::gs_host_frame>* tx_can_queue_{nullptr};
  IQueue<unav::io::usb::UsbCan::gs_host_frame>* rx_can_queue_{nullptr};
  Thread<OC_STACK_SIZE> can_task;

  inline void can_comm_task_function() {
    const uint32_t ONE_SECOND = 1e6;
    tickstime_t last_heartbeat = Timing::get_ticks();

    while (true) {
      handle_tx_rx();
      if (Timing::get_us_since(last_heartbeat) > ONE_SECOND) {
        canard_.send_heartbeat(false);
        last_heartbeat = Timing::get_ticks();
      }

      vTaskDelay(0);
    }
  }

  void handle_tx_rx() {
    static unav::io::usb::UsbCan::gs_host_frame can_frame{};
    static CanardRxTransfer transfer = {};
    static CanardFrame frame = {};
    // TX
    auto tqi = canard_.tx_peek();  // Find the highest-priority frame.
    while (tqi != nullptr) {
      // Attempt transmission only if the frame is not yet timed out while waiting in the TX queue.
      // Otherwise just drop it and move on to the next one.
      if ((tqi->tx_deadline_usec == 0) || (tqi->tx_deadline_usec > Timing::get_us())) {
        auto frame = &tqi->frame;
        can_frame.can_id = frame->extended_can_id | CAN_EFF_FLAG;
        // tx_can_frame.flags = GS_CAN_FLAG_BRS;
        can_frame.echo_id = 0xFFFFFFFF;
        can_frame.timestamp_us = Timing::get_us();
        can_frame.data.data64 = *static_cast<const uint64_t*>(frame->payload);
        tx_can_queue_->send(can_frame, 0);
      };
      canard_.memory_free(canard_.tx_pop(tqi));
      tqi = canard_.tx_peek();
    }

    // RX
    while (rx_can_queue_->messages_waiting() > 0) {
      if (!rx_can_queue_->receive(can_frame, 1)) {
        continue;
      }
      const bool valid = ((can_frame.can_id & CAN_EFF_FLAG) != 0) &&  // Extended frame
                         ((can_frame.can_id & CAN_ERR_FLAG) == 0) &&  // Not RTR frame
                         ((can_frame.can_id & CAN_RTR_FLAG) == 0);    // Not error frame
      if (!valid) {
        continue;  // Not an extended data frame -- drop silently and return early.
      }

      frame.extended_can_id = can_frame.can_id & CAN_EFF_MASK;
      frame.payload_size = sizeof(can_frame.data);
      frame.payload = &can_frame.data.data[0];

      const int8_t canard_result = canard_.rx_accept(&frame, &transfer, nullptr);
      if (canard_result > 0) {
        canard_.process_rx_transfer(&transfer);
        canard_.memory_free(transfer.payload);

        // The frame did not complete a transfer so there is nothing to do.
        // OOM should never occur if the heap is sized correctly. You can track OOM errors via heap API.
      } else if (!((canard_result == 0) || (canard_result == -CANARD_ERROR_OUT_OF_MEMORY))) {
        Error_Handler();  // No other error can possibly occur at runtime.
      }
    }
  }
};

}  // namespace unav::opencyphal
#endif  // APPLICATION_H