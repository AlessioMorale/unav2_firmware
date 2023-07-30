#pragma once
#include <etl/vector.h>

#include <cstddef>
#ifndef CANARD_WRAPPER_H_
#define CANARD_WRAPPER_H_
#include <canard.h>
#include <debug.h>
#include <etl/array.h>
#include <etl/delegate.h>
#include <etl/flat_map.h>
#include <o1heap.h>
#include <timing.h>
#include <uavcan/node/Heartbeat_1_0.h>
#include <uavcan/node/Version_1_0.h>
#include <uavcan/pnp/NodeIDAllocationData_2_0.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>

#define CAN_REDUNDANCY_FACTOR 1
#define CAN_TX_QUEUE_CAPACITY 20
#define MAX_PORTS 10

namespace unav::opencyphal {

extern "C" {
static auto canard_allocate(CanardInstance* const ins, const size_t amount) -> void*;
static void canard_free(CanardInstance* const ins, void* const pointer);
}

class Canard {
 private:
  using subscription_t = struct {
    CanardPortID port;
    CanardRxSubscription subscription;
    etl::delegate<void(const CanardRxTransfer&)> callback;
  };
  static constexpr size_t MAX_SUBSCRIPTIONS = 20;
  etl::flat_map<CanardPortID, subscription_t, MAX_SUBSCRIPTIONS> subscriptions_{};
  CanardInstance canard_;
  uavcan_node_Heartbeat_1_0 heartbeat{};
  alignas(O1HEAP_ALIGNMENT) std::array<uint8_t, 1024 * 2> heap_arena_{};
  uint32_t uavcan_node_heartbeat_counter{};

  const uavcan_node_Version_1_0 version_{0, 1};
  CanardMicrosecond started_at_;
  CanardTxQueue tx_queue_;
  etl::array<uint8_t, 16> node_unique_id = {0};

 public:
  CanardMicrosecond get_timestamp() { return (CanardMicrosecond)Timing::get_us(); }

  void setup_canard(etl::array<uint8_t, 16> unique_id) {
    started_at_ = get_timestamp();
    node_unique_id = unique_id;
    canard_ = canardInit(&canard_allocate, &canard_free);
    canard_.user_reference = this;
    canard_.node_id = CANARD_NODE_ID_UNSET;
    tx_queue_ = canardTxInit(CAN_TX_QUEUE_CAPACITY, CANARD_MTU_CAN_FD);
    heap_ = o1heapInit(heap_arena_.data(), heap_arena_.size());

    subscribe(CanardTransferKindMessage, uavcan_pnp_NodeIDAllocationData_2_0_FIXED_PORT_ID_,
              uavcan_pnp_NodeIDAllocationData_2_0_EXTENT_BYTES_, CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
              etl::delegate<void(const CanardRxTransfer&)>::create<Canard, &Canard::update_node_id>(*this));
  }

  void subscribe(const CanardTransferKind transfer_kind, const CanardPortID port_id, const size_t extent,
                 const CanardMicrosecond transfer_id_timeout_usec,
                 etl::delegate<void(const CanardRxTransfer&)> callback) {
    auto subscription = &subscriptions_[port_id];

    subscription->port = port_id;
    subscription->callback = callback;

    (void)canardRxSubscribe(&canard_, transfer_kind, port_id, extent, transfer_id_timeout_usec,
                            &subscription->subscription);
  }

  bool node_is_anonymous() { return canard_.node_id == CANARD_NODE_ID_UNSET; }

  void send_heartbeat(bool warnings = false) {
    if (!node_is_anonymous()) {
      uint8_t serialized[uavcan_node_Heartbeat_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {};
      const O1HeapDiagnostics heap_diag = o1heapGetDiagnostics(heap_);
      CanardTransferMetadata meta = {
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
    } else {
      handle_node_allocation();
    }
  }

  void handle_node_allocation() {
    static uint8_t uavcan_pnp_allocation = 0;
    if (std::rand() > RAND_MAX / 2) {
      // Note that this will only work over CAN FD. If you need to run PnP over Classic CAN, use message v1.0.
      uavcan_pnp_NodeIDAllocationData_2_0 msg = {0};
      msg.node_id.value = UINT16_MAX;
      std::memcpy(&msg.unique_id, node_unique_id.data(), std::min(node_unique_id.size(), sizeof(msg.unique_id)));
      uint8_t serialized[uavcan_pnp_NodeIDAllocationData_2_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
      size_t serialized_size = sizeof(serialized);
      const int8_t err = uavcan_pnp_NodeIDAllocationData_2_0_serialize_(&msg, &serialized[0], &serialized_size);
      assert(err >= 0);
      if (err >= 0) {
        const CanardTransferMetadata transfer = {
            .priority = CanardPrioritySlow,
            .transfer_kind = CanardTransferKindMessage,
            .port_id = uavcan_pnp_NodeIDAllocationData_2_0_FIXED_PORT_ID_,
            .remote_node_id = CANARD_NODE_ID_UNSET,
            .transfer_id = (CanardTransferID)(uavcan_pnp_allocation++),
        };
        send(Timing::get_us() + 1e6, &transfer, serialized_size, &serialized[0]);
      }
    }
  }

  void send(const uint32_t tx_deadline_usec, const CanardTransferMetadata* const metadata, const size_t payload_size,
            const void* const payload) {
    (void)canardTxPush(&tx_queue_, &canard_, tx_deadline_usec, metadata, payload_size, payload);
  }

  void process_rx_transfer(const CanardRxTransfer* const transfer) {
    auto sub = subscriptions_.find(transfer->metadata.port_id);
    if (sub != subscriptions_.end()) {
      sub->second.callback.call_if(*transfer);
    }
  }

  void update_node_id(const CanardRxTransfer& transfer) {
    size_t size = transfer.payload_size;
    if (transfer.metadata.port_id == uavcan_pnp_NodeIDAllocationData_2_0_FIXED_PORT_ID_) {
      uavcan_pnp_NodeIDAllocationData_2_0 msg = {0};
      if (uavcan_pnp_NodeIDAllocationData_2_0_deserialize_(&msg, static_cast<uint8_t*>(transfer.payload), &size) >= 0) {
        logger_debug("update_node_id %u\r\n", msg.node_id.value);
        canard_.node_id = msg.node_id.value;
      }
    }
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