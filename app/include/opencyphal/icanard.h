#pragma once
#ifndef ICANARD_H_
#define ICANARD_H_
#include <etl/array.h>
#include <etl/delegate.h>
#include "canard.h"

using message_callback = etl::delegate<void(const CanardRxTransfer&)>;
class ICanard {
 public:
  virtual CanardMicrosecond get_timestamp() = 0;

  virtual void setup_canard(etl::array<uint8_t, 16>& unique_id) = 0;

  virtual void subscribe(const CanardTransferKind transfer_kind, const CanardPortID port_id, const size_t extent,
                         const CanardMicrosecond transfer_id_timeout_usec, message_callback callback) = 0;
  virtual void send_heartbeat(bool warnings = false) = 0;

  virtual void send(const uint32_t tx_deadline_usec, const CanardTransferMetadata* const metadata,
                    const size_t payload_size, const void* const payload) = 0;

  virtual void send_response(const CanardRxTransfer* const original_request_transfer, const size_t payload_size,
                             const void* const payload) = 0;
  virtual void process_rx_transfer(const CanardRxTransfer* const transfer) = 0;

  virtual const CanardTxQueueItem* tx_peek() = 0;
  virtual const CanardTxQueueItem* tx_pop(const CanardTxQueueItem* const item) = 0;
  virtual void memory_free(const void* const pointer) = 0;
  virtual void set_node_id(uint16_t id) = 0;
 private:
};

#endif  // ICANARD_H_