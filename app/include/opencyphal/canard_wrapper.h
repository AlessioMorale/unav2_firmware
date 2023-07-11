#pragma once
#ifndef CANARD_WRAPPER_H_
#define CANARD_WRAPPER_H_
#include <canard.h>
#include <debug.h>
#include <o1heap.h>
#include <timing.h>
#include <uavcan/node/Heartbeat_1_0.h>
#include <uavcan/node/Version_1_0.h>
#include <uavcan/pnp/NodeIDAllocationData_2_0.h>

#include <array>

#define CAN_REDUNDANCY_FACTOR 1
/// For CAN FD the queue can be smaller.
#define CAN_TX_QUEUE_CAPACITY 100

namespace unav::opencyphal {

extern "C" {
static auto canard_allocate(CanardInstance* const ins, const size_t amount) -> void*;
static void canard_free(CanardInstance* const ins, void* const pointer);
}

class Canard {
 private:
  CanardInstance canard_;
  uavcan_node_Heartbeat_1_0 heartbeat = {};
  uint8_t serialized[uavcan_node_Heartbeat_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {};
  alignas(O1HEAP_ALIGNMENT) std::array<uint8_t, 1024 * 2> heap_arena_{};
  uint32_t uavcan_node_heartbeat_counter = {};

  uavcan_node_Version_1_0 version_;
  CanardMicrosecond started_at_;
  CanardTxQueue tx_queue_;

 public:
  CanardMicrosecond get_timestamp() { return (CanardMicrosecond)Timing::get_us(); }
  inline void setup_canard() {
    static CanardRxSubscription rx;
    started_at_ = Timing::get_us();
    canard_ = canardInit(&canard_allocate, &canard_free);
    canard_.user_reference = this;
    canard_.node_id = 42;  //  CANARD_NODE_ID_UNSET;
    tx_queue_ = canardTxInit(20, CANARD_MTU_CAN_CLASSIC);
    heap_ = o1heapInit(heap_arena_.data(), heap_arena_.size());

    canardRxSubscribe(&canard_, CanardTransferKindMessage, uavcan_pnp_NodeIDAllocationData_2_0_FIXED_PORT_ID_,
                      uavcan_pnp_NodeIDAllocationData_2_0_EXTENT_BYTES_, CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC, &rx);
  }

  void send_heartbeat(bool warnings = false) {
    static const O1HeapDiagnostics heap_diag = o1heapGetDiagnostics(heap_);
    static CanardTransferMetadata meta = {
        .priority = CanardPriorityNominal,
        .transfer_kind = CanardTransferKindMessage,
        .port_id = uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_,
        .remote_node_id = CANARD_NODE_ID_UNSET,
    };
    heartbeat.uptime = (uint32_t)Timing::get_us() / 1e6;
    heartbeat.mode.value = uavcan_node_Mode_1_0_OPERATIONAL;
    if (heap_diag.oom_count > 0 || warnings) {
      heartbeat.health.value = uavcan_node_Health_1_0_CAUTION;
    } else {
      heartbeat.health.value = uavcan_node_Health_1_0_NOMINAL;
    }

    size_t serialized_size = sizeof(serialized);
    const int8_t err = uavcan_node_Heartbeat_1_0_serialize_(&heartbeat, serialized, &serialized_size);
    if (err < 0) {
      Error_Handler();
    }
    if (err >= 0) {
      meta.transfer_id = (CanardTransferID)(uavcan_node_heartbeat_counter++),
      send(Timing::get_us() + 1e6,  // Set transmission deadline 1 second, optimal for heartbeat.
           &meta, serialized_size, &serialized[0]);
    }
  }

  void send(const uint32_t tx_deadline_usec, const CanardTransferMetadata* const metadata, const size_t payload_size,
            const void* const payload) {
    (void)canardTxPush(&tx_queue_, &canard_, tx_deadline_usec, metadata, payload_size, payload);
  }

  void process_rx_transfer(const CanardRxTransfer* const transfer) {
    logger_debug("transfer k%u: p%u\r\n", transfer->metadata.transfer_kind, transfer->metadata.port_id);
  }

  const CanardTxQueueItem* tx_peek() { return canardTxPeek(&tx_queue_); }
  const CanardTxQueueItem* tx_pop(const CanardTxQueueItem* const item) { return canardTxPop(&tx_queue_, item); }

  const int8_t rx_accept(const CanardFrame* const frame, CanardRxTransfer* const transfer,
                         CanardRxSubscription** const subscription = nullptr) {
    return canardRxAccept(&canard_, get_timestamp(), frame, 0, transfer, subscription);
  }

  void memory_free(const void* const pointer) { canard_.memory_free(&this->canard_, const_cast<void* const>(pointer)); }

 private:
  friend auto canard_allocate(CanardInstance* const, const size_t) -> void*;
  friend void canard_free(CanardInstance* const, void* const);

 protected:
  O1HeapInstance* heap_{nullptr};
};

extern "C" {

inline static void* canard_allocate(CanardInstance* const ins, const size_t amount) {
  O1HeapInstance* const heap = ((Canard*)ins->user_reference)->heap_;
  assert(o1heapDoInvariantsHold(heap));
  return o1heapAllocate(heap, amount);
}

inline static void canard_free(CanardInstance* const ins, void* const pointer) {
  O1HeapInstance* const heap = ((Canard*)ins->user_reference)->heap_;
  o1heapFree(heap, pointer);
}
}
}  // namespace unav::opencyphal
#endif