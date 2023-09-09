#pragma once
#include <etl/vector.h>

#include <cstddef>
#ifndef CANARD_WRAPPER_H_
#define CANARD_WRAPPER_H_
#include <canard.h>
#include <etl/array.h>
#include <etl/delegate.h>
#include <etl/flat_map.h>
#include <o1heap.h>
#include <timing.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>

#include "cyphal_messages_handlers.h"

#define CAN_REDUNDANCY_FACTOR 1
#define CAN_TX_QUEUE_CAPACITY 20
#define MAX_PORTS 10

namespace unav::opencyphal {

extern "C" {
static auto canard_allocate(CanardInstance* const ins, const size_t amount) -> void*;
static void canard_free(CanardInstance* const ins, void* const pointer);
}

class Canard : public ICanard {
 private:
  using subscription_t = struct {
    CanardPortID port;
    CanardRxSubscription subscription;
    message_callback callback;
  };

  CyphalMessagesHandlers handlers_{*this};
  static constexpr size_t MAX_SUBSCRIPTIONS = 20;
  etl::flat_map<CanardPortID, subscription_t, MAX_SUBSCRIPTIONS> subscriptions_{};
  CanardInstance canard_;
  alignas(O1HEAP_ALIGNMENT) std::array<uint8_t, 1024 * 2> heap_arena_{};

  const uavcan_node_Version_1_0 version_{0, 1};
  CanardMicrosecond started_at_;
  CanardTxQueue tx_queue_;

  bool node_is_anonymous() { return canard_.node_id == CANARD_NODE_ID_UNSET; }

  friend auto canard_allocate(CanardInstance* const, const size_t) -> void*;
  friend void canard_free(CanardInstance* const, void* const);

 public:
  Canard() = default;

  auto get_timestamp() -> CanardMicrosecond override { return (CanardMicrosecond)Timing::get_us(); }

  void setup_canard(etl::array<uint8_t, 16>& unique_id) override {
    started_at_ = get_timestamp();
    canard_ = canardInit(&canard_allocate, &canard_free);
    canard_.user_reference = this;
    canard_.node_id = CANARD_NODE_ID_UNSET;
    tx_queue_ = canardTxInit(CAN_TX_QUEUE_CAPACITY, CANARD_MTU_CAN_FD);
    heap_ = o1heapInit(heap_arena_.data(), heap_arena_.size());

    handlers_.set_unique_id(unique_id);
    handlers_.register_node_allocation();
    handlers_.register_get_info_request();
  }

  void subscribe(const CanardTransferKind transfer_kind, const CanardPortID port_id, const size_t extent,
                 const CanardMicrosecond transfer_id_timeout_usec, message_callback callback) override {
    auto subscription = &subscriptions_[port_id];

    subscription->port = port_id;
    subscription->callback = callback;

    (void)canardRxSubscribe(&canard_, transfer_kind, port_id, extent, transfer_id_timeout_usec,
                            &subscription->subscription);
  }

  void send_heartbeat(bool warnings = false) override {
    if (!node_is_anonymous()) {
      const O1HeapDiagnostics heap_diag = o1heapGetDiagnostics(heap_);
      handlers_.send_heartbeat(heap_diag.oom_count > 0 || warnings);
    } else if (std::rand() > RAND_MAX / 2) {
      handlers_.request_node_allocation();
    }
  }

  void send(const uint32_t tx_deadline_usec, const CanardTransferMetadata* const metadata, const size_t payload_size,
            const void* const payload) override {
    (void)canardTxPush(&tx_queue_, &canard_, tx_deadline_usec, metadata, payload_size, payload);
  }

  void send_response(const CanardRxTransfer* const original_request_transfer, const size_t payload_size,
                     const void* const payload) override {
    CanardTransferMetadata meta = original_request_transfer->metadata;
    meta.transfer_kind = CanardTransferKindResponse;
    send(original_request_transfer->timestamp_usec + 1e6, &meta, payload_size, payload);
  }

  void process_rx_transfer(const CanardRxTransfer* const transfer) override {
    auto sub = subscriptions_.find(transfer->metadata.port_id);
    if (sub != subscriptions_.end()) {
      sub->second.callback.call_if(*transfer);
    }
  }

  auto tx_peek() -> const CanardTxQueueItem* override { return canardTxPeek(&tx_queue_); }
  auto tx_pop(const CanardTxQueueItem* const item) -> const CanardTxQueueItem* override {
    return canardTxPop(&tx_queue_, item);
  }

  auto rx_accept(const CanardFrame* const frame, CanardRxTransfer* const transfer,
                 CanardRxSubscription** const subscription = nullptr) -> const int8_t {
    return canardRxAccept(&canard_, get_timestamp(), frame, 0, transfer, subscription);
  }

  void memory_free(const void* const pointer) override {
    canard_.memory_free(&this->canard_, const_cast<void* const>(pointer));
  }

  void set_node_id(uint16_t id) override { canard_.node_id = id; }

 protected:
  O1HeapInstance* heap_{nullptr};
};

extern "C" {

inline static auto canard_allocate(CanardInstance* const ins, const size_t amount) -> void* {
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