#pragma once
#ifndef CYPHAL_MESSAGES_HANDLERS_
#define CYPHAL_MESSAGES_HANDLERS_
#include <canard.h>
#include <debug.h>
#include <timing.h>
#include <uavcan/node/GetInfo_1_0.h>
#include <uavcan/node/Heartbeat_1_0.h>
#include <uavcan/node/Version_1_0.h>
#include <uavcan/pnp/NodeIDAllocationData_2_0.h>

#include <cstring>

#include "icanard.h"
namespace unav::opencyphal {

class CyphalMessagesHandlers {
 public:
  CyphalMessagesHandlers(ICanard& canard) : canard_{&canard} {}

  void set_unique_id(etl::array<uint8_t, 16>& node_unique_id) { node_unique_id_ = node_unique_id; }

  void register_node_allocation() {
    canard_->subscribe(
        CanardTransferKindMessage, uavcan_pnp_NodeIDAllocationData_2_0_FIXED_PORT_ID_,
        uavcan_pnp_NodeIDAllocationData_2_0_EXTENT_BYTES_, CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
        message_callback::create<CyphalMessagesHandlers, &CyphalMessagesHandlers::handle_node_allocation>(*this));
  }
  void register_get_info_request() {
    canard_->subscribe(
        CanardTransferKindRequest, uavcan_node_GetInfo_1_0_FIXED_PORT_ID_,
        uavcan_node_GetInfo_Request_1_0_EXTENT_BYTES_, CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
        message_callback::create<CyphalMessagesHandlers, &CyphalMessagesHandlers::handle_get_info_request>(*this));
  }

  void send_heartbeat(bool warnings) {
    uint8_t serialized[uavcan_node_Heartbeat_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {};
    CanardTransferMetadata meta = {
        .priority = CanardPriorityNominal,
        .transfer_kind = CanardTransferKindMessage,
        .port_id = uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_,
        .remote_node_id = CANARD_NODE_ID_UNSET,
    };
    heartbeat.uptime = Timing::get_ms() / 1e3;
    heartbeat.mode.value = uavcan_node_Mode_1_0_OPERATIONAL;
    if (warnings) {
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
      canard_->send(Timing::get_us() + 1e6,  // Set transmission deadline 1 second, optimal for heartbeat.
                    &meta, serialized_size, &serialized[0]);
    }
  }

  void request_node_allocation() {
    static uint8_t uavcan_pnp_allocation = 0;
    // Note that this will only work over CAN FD. If you need to run PnP over Classic CAN, use message v1.0.
    uavcan_pnp_NodeIDAllocationData_2_0 msg = {0};
    msg.node_id.value = UINT16_MAX;
    std::memcpy(&msg.unique_id, node_unique_id_.data(), std::min(node_unique_id_.size(), sizeof(msg.unique_id)));
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
      canard_->send(Timing::get_us() + 1e6, &transfer, serialized_size, &serialized[0]);
    }
  }

 private:
  ICanard* canard_;
  uavcan_node_Heartbeat_1_0 heartbeat{};
  uint32_t uavcan_node_heartbeat_counter{};
  etl::array<uint8_t, 16> node_unique_id_{0};

  void handle_get_info_request(const CanardRxTransfer& transfer) {
    // The request object is empty so we don't bother deserializing it. Just send the response.
    uavcan_node_GetInfo_Response_1_0 resp{};
    resp.protocol_version.major = CANARD_CYPHAL_SPECIFICATION_VERSION_MAJOR;
    resp.protocol_version.minor = CANARD_CYPHAL_SPECIFICATION_VERSION_MINOR;

    // The hardware version is not populated in this demo because it runs on no specific hardware.
    // An embedded node like a servo would usually determine the version by querying the hardware.

    resp.software_version.major = VERSION_MAJOR;
    resp.software_version.minor = VERSION_MINOR;
    resp.software_vcs_revision_id = VCS_REVISION_ID;
    std::memcpy(&resp.unique_id, node_unique_id_.data(), std::min(node_unique_id_.size(), sizeof(resp.unique_id)));

    // The node name is the name of the product like a reversed Internet domain name (or like a Java package).
    resp.name.count = strlen(NODE_NAME);
    std::memcpy(&resp.name.elements, NODE_NAME, resp.name.count);

    uint8_t serialized[uavcan_node_GetInfo_Response_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
    size_t serialized_size = sizeof(serialized);
    const int8_t res = uavcan_node_GetInfo_Response_1_0_serialize_(&resp, &serialized[0], &serialized_size);
    if (res >= 0) {
      canard_->send_response(&transfer, serialized_size, &serialized[0]);
    } else {
      Error_Handler();
    }
  }

  void handle_node_allocation(const CanardRxTransfer& transfer) {
    size_t size = transfer.payload_size;
    uavcan_pnp_NodeIDAllocationData_2_0 msg = {0};
    if (uavcan_pnp_NodeIDAllocationData_2_0_deserialize_(&msg, static_cast<uint8_t*>(transfer.payload), &size) >= 0) {
      logger_debug("update_node_id %u\r\n", msg.node_id.value);
      canard_->set_node_id(msg.node_id.value);
    }
  }
};
}  // namespace unav::opencyphal
#endif  // CYPHAL_MESSAGES_HANDLERS_