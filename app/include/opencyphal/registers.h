#pragma once
#ifndef __REGISTERS_H__
#define __REGISTERS_H__
#include <uavcan/node/port/List_0_1.h>
#include <uavcan/_register/Access_1_0.h>
#include <uavcan/_register/List_1_0.h>
#include <etl/flat_map.h>
#include <canard.h>

class Registers{

void register_get_info_request() {
    canard_->subscribe(
        CanardTransferKindRequest, uavcan_node_GetInfo_1_0_FIXED_PORT_ID_,
        uavcan_node_GetInfo_Request_1_0_EXTENT_BYTES_, CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
        message_callback::create<CyphalMessagesHandlers, &CyphalMessagesHandlers::handle_get_info_request>(*this));
  }
void handle_register_access()
{
            uavcan_register_Access_Request_1_0 req  = {0};
            size_t                             size = transfer->payload_size;
            if (uavcan_register_Access_Request_1_0_deserialize_(&req, transfer->payload, &size) >= 0)
            {
                const uavcan_register_Access_Response_1_0 resp = processRequestRegisterAccess(&req);
                uint8_t serialized[uavcan_register_Access_Response_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                size_t  serialized_size = sizeof(serialized);
                if (uavcan_register_Access_Response_1_0_serialize_(&resp, &serialized[0], &serialized_size) >= 0)
                {
                    sendResponse(state,
                                 transfer->timestamp_usec + MEGA,
                                 &transfer->metadata,
                                 serialized_size,
                                 &serialized[0]);
                }
            }
        }

}





#endif // __REGISTERS_H__