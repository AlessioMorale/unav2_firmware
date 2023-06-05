#pragma once
#ifndef OPENCYPHAL_H
#define OPENCYPHAL_H
#include <FreeRTOS.h>
#include <canard.h>
#include <debug.h>
#include <io/usb/gs_can/gs_usb.h>
#include <o1heap.h>
#include <uavcan/_register/Access_1_0.h>
#include <uavcan/_register/List_1_0.h>
#include <uavcan/node/ExecuteCommand_1_1.h>
#include <uavcan/node/GetInfo_1_0.h>
#include <uavcan/node/Heartbeat_1_0.h>
#include <uavcan/node/Version_1_0.h>
#include <uavcan/node/port/List_0_1.h>
#include <uavcan/pnp/NodeIDAllocationData_2_0.h>

#include <memory>

namespace unav::opencyphal {

#define KILO 1000L
#define MEGA ((int64_t)KILO * KILO)

#define CAN_REDUNDANCY_FACTOR 1
/// For CAN FD the queue can be smaller.
#define CAN_TX_QUEUE_CAPACITY 100
#define OC_STACK_SIZE configMINIMAL_STACK_SIZE

extern "C" {
static void* canardAllocate(CanardInstance* const ins, const size_t amount);
static void canardFree(CanardInstance* const ins, void* const pointer);
}

class OpenCyphal {
 public:
  void setup(IQueue<unav::io::usb::UsbCan::gs_host_frame>& tx_queue,
             IQueue<unav::io::usb::UsbCan::gs_host_frame>& rx_queue) {
    logger_debug("OpenCyphal.Setup()!\r\n");
    setup_canard();
    tx_can_queue_ = &tx_queue;
    rx_can_queue_ = &rx_queue;

    can_task.create();
  };

  OpenCyphal()
      : heap_{nullptr},
        heap_arena_{0},
        tx_can_queue_{nullptr},
        rx_can_queue_{nullptr},
        can_task{thread_delegate::create<OpenCyphal, &OpenCyphal::can_comm_task_function>(*this), ThreadPriority::High,
                 "cyph/com"} {

        };

 protected:
  O1HeapInstance* heap_;

 private:
  CanardInstance canard_;
  _Alignas(O1HEAP_ALIGNMENT) uint8_t heap_arena_[1024 * 8];
  uavcan_node_Version_1_0 version_;
  CanardMicrosecond started_at_;
  CanardTxQueue tx_queue_;
  IQueue<unav::io::usb::UsbCan::gs_host_frame>* tx_can_queue_;
  IQueue<unav::io::usb::UsbCan::gs_host_frame>* rx_can_queue_;
  Thread<OC_STACK_SIZE> can_task;

  inline void can_comm_task_function() {
    unav::io::usb::UsbCan::gs_host_frame tx_can_frame{0};
    unav::io::usb::UsbCan::gs_host_frame rx_can_frame{0};

    while (true) {
      // TX
      const CanardTxQueueItem* tqi = canardTxPeek(&tx_queue_);  // Find the highest-priority frame.
      while (tqi != NULL) {
        // Attempt transmission only if the frame is not yet timed out while waiting in the TX queue.
        // Otherwise just drop it and move on to the next one.
        if ((tqi->tx_deadline_usec == 0) || (tqi->tx_deadline_usec > Timing::get_us())) {
          auto frame = &tqi->frame;
          tx_can_frame.can_id = frame->extended_can_id | CAN_EFF_FLAG;
          tx_can_frame.flags = GS_CAN_FLAG_BRS;
          (void)memcpy(tx_can_frame.data, frame->payload, frame->payload_size);
          tx_can_queue_->send(tx_can_frame, 0);
        };
        canard_.memory_free(&canard_, canardTxPop(&tx_queue_, tqi));
        tqi = canardTxPeek(&tx_queue_);
      }
      // RX

      const CanardMicrosecond timestamp_usec = (CanardMicrosecond)Timing::get_us();
      while (rx_can_queue_->receive(rx_can_frame, 0)) {
        const bool valid = ((rx_can_frame.can_id & CAN_EFF_FLAG) != 0) &&  // Extended frame
                           ((rx_can_frame.can_id & CAN_ERR_FLAG) == 0) &&  // Not RTR frame
                           ((rx_can_frame.can_id & CAN_RTR_FLAG) == 0);    // Not error frame
        if (!valid) {
          continue;  // Not an extended data frame -- drop silently and return early.
        }

        CanardRxTransfer transfer = {};
        CanardFrame frame = {0};
        frame.extended_can_id = rx_can_frame.can_id & CAN_EFF_MASK;
        frame.payload_size = sizeof(rx_can_frame.data);
        frame.payload = &rx_can_frame.data[0];

        const int8_t canard_result = canardRxAccept(&canard_, timestamp_usec, &frame, 0, &transfer, NULL);
        if (canard_result > 0) {
          processReceivedTransfer(&transfer);
          canard_.memory_free(&canard_, (void*)transfer.payload);

          // The frame did not complete a transfer so there is nothing to do.
          // OOM should never occur if the heap is sized correctly. You can track OOM errors via heap API.
        } else if (!((canard_result == 0) || (canard_result == -CANARD_ERROR_OUT_OF_MEMORY))) {
          assert(false);  // No other error can possibly occur at runtime.
        }
      }
      vTaskDelay(1);
    }
  }

  inline void setup_canard() {
    static CanardRxSubscription rx;

    canard_ = canardInit(&canardAllocate, &canardFree);
    canard_.user_reference = this;
    canard_.node_id = CANARD_NODE_ID_UNSET;
    tx_queue_ = canardTxInit(20, CANARD_MTU_CAN_CLASSIC);
    heap_ = o1heapInit(heap_arena_, sizeof(heap_arena_));

    const int8_t res = canardRxSubscribe(
        &canard_, CanardTransferKindMessage, uavcan_pnp_NodeIDAllocationData_2_0_FIXED_PORT_ID_,
        uavcan_pnp_NodeIDAllocationData_2_0_EXTENT_BYTES_, CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC, &rx);
  }

  void processReceivedTransfer(const CanardRxTransfer* const transfer) {
    logger_debug("transfer k%u: p%u\r\n", transfer->metadata.transfer_kind, transfer->metadata.port_id);
  }

  friend void* canardAllocate(CanardInstance* const, const size_t);
  friend void canardFree(CanardInstance* const, void* const);
};
extern "C" {

inline static void* canardAllocate(CanardInstance* const ins, const size_t amount) {
  O1HeapInstance* const heap = ((OpenCyphal*)ins->user_reference)->heap_;
  assert(o1heapDoInvariantsHold(heap));
  return o1heapAllocate(heap, amount);
}

inline static void canardFree(CanardInstance* const ins, void* const pointer) {
  O1HeapInstance* const heap = ((OpenCyphal*)ins->user_reference)->heap_;
  o1heapFree(heap, pointer);
}
}
}  // namespace unav::opencyphal
#endif  // APPLICATION_H