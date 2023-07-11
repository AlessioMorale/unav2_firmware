#pragma once
#ifndef CDC_H
#define CDC_H
#include <bsp/board.h>
#include <debug.h>
#include <etl/algorithm.h>
#include <etl/array.h>
#include <stm32f4xx.h>
#include <stm32f4xx_hal.h>
#include <tusb.h>
#include <wrappers/queue.h>
#include <wrappers/thread.h>

#include <cstddef>
#include <memory>
#include <string>

#include "gs_usb_private.h"
#include "tusb_config.h"

using namespace freertos::wrappers;

namespace unav::io::usb {

#define USBD_STACK_SIZE (3 * configMINIMAL_STACK_SIZE / 2) * (CFG_TUSB_DEBUG ? 2 : 1)
#define GS_STACK_SIZE configMINIMAL_STACK_SIZE
#define DATA_QUEUE_SIZE 10
#define CAN_CLOCK_SPEED 42000000
class UsbCan {
 private:
  struct gs_control_message {
    gs_usb_breq request_type;
    const void *data;
    size_t length;
    uint8_t direction;
  };
  inline static std::unique_ptr<UsbCan> instance;

 public:
  struct gs_host_frame {
    uint32_t echo_id;
    uint32_t can_id;

    uint8_t can_dlc;
    uint8_t channel;
    uint8_t flags;
    uint8_t reserved;
    union data_u {
      uint8_t data[8];
      uint64_t data64;
    } data;
    uint32_t timestamp_us;

  } __packed __aligned(4);

  struct gs_host_frame_canfd {
    u32 echo_id;
    u32 can_id;

    u8 can_dlc;
    u8 channel;
    u8 flags;
    u8 reserved;

    u8 data[64];
  } __packed __aligned(4);

  static UsbCan *get_instance() {
    if (instance == nullptr) {
      instance.reset(new UsbCan());
    }
    return instance.get();
  }

  IQueue<gs_host_frame> &get_data_in_queue() { return data_in_queue; }
  IQueue<gs_host_frame> &get_data_out_queue() { return data_out_queue; }
  /// @brief setup the cdc stack
  void setup() {
    // initialise the BSP
    unav::bsp::Board::instance.init_usb_hw();

    // Create a task for tinyusb device stack
    usb_device_task.create();
    // Create CDC task
    usb_can_task.create();
  }

  bool control_xfer_cb(uint8_t rhport, uint8_t stage, const tusb_control_request_t *request) {
    logger_debug("control_xfer_cb %u\r\n", stage);

    if (request->bmRequestType_bit.type != TUSB_REQ_TYPE_VENDOR || request->wIndex != 0) {
      return true;
    }

    auto control_message = etl::find_if(  //
        control_messages.begin(),         //
        control_messages.end(),           //
        [request](auto i) { return static_cast<uint8_t>(i.request_type) == request->bRequest; });

    if (static_cast<uint8_t>(control_message->request_type) == request->bRequest) {
      if (stage == CONTROL_STAGE_SETUP) {
        return control_message->length == request->wLength &&
               tud_control_xfer(rhport, request, const_cast<void *>(control_message->data), control_message->length);
      }

      if (request->bmRequestType_bit.direction == TUSB_DIR_OUT) {
        switch (stage) {
          case CONTROL_STAGE_DATA:
            return apply_config(request->bRequest, request->wValue);
          case CONTROL_STAGE_ACK:
            return true;
        }
      } else {
        return true;
      }
    }
    return false;
  }

 private:
  UsbCan()
      : data_in_queue("gs_usb_in"),
        data_out_queue("gs_usb_out"),
        usb_device_task{thread_delegate::create<UsbCan, &UsbCan::usb_device_task_function>(*this),
                        ThreadPriority::Highest, "usb/dev"},
        usb_can_task{thread_delegate::create<UsbCan, &UsbCan::usb_can_task_function>(*this), ThreadPriority::High,
                     "usb/gs_usb"} {}

  Queue<DATA_QUEUE_SIZE, gs_host_frame> data_in_queue;
  Queue<DATA_QUEUE_SIZE, gs_host_frame> data_out_queue;

  Thread<USBD_STACK_SIZE> usb_device_task;
  Thread<GS_STACK_SIZE> usb_can_task;

  const struct gs_device_config device_config { .icount = 0, .sw_version = 2, .hw_version = 1, };

  const struct gs_device_bt_const device_bt_const = {
      // supported features
      GS_CAN_FEATURE_LISTEN_ONLY         //
          | GS_CAN_FEATURE_LOOP_BACK     //
          | GS_CAN_FEATURE_HW_TIMESTAMP  //
          | GS_CAN_FEATURE_IDENTIFY      //
          | GS_CAN_FEATURE_PAD_PKTS_TO_MAX_PKT_SIZE,
      CAN_CLOCK_SPEED,  // can timing base clock
      1,                // tseg1 min
      16,               // tseg1 max
      1,                // tseg2 min
      8,                // tseg2 max
      4,                // sjw max
      1,                // brp min
      1024,             // brp_max
      1,                // brp increment;
  };
  uint32_t byte_order = 0;
  struct gs_device_bittiming device_bittiming;
  struct gs_device_mode device_mode;

  // gs_control_message ccontrol_messages[5]{
  etl::array<gs_control_message, 5> control_messages = {
      {{gs_usb_breq::GS_USB_BREQ_DEVICE_CONFIG, &device_config, sizeof(device_config), TUSB_DIR_IN},
       {gs_usb_breq::GS_USB_BREQ_BT_CONST, &device_bt_const, sizeof(device_bt_const), TUSB_DIR_IN},
       {gs_usb_breq::GS_USB_BREQ_HOST_FORMAT, &byte_order, sizeof(byte_order), TUSB_DIR_OUT},
       {gs_usb_breq::GS_USB_BREQ_BITTIMING, &device_bittiming, sizeof(device_bittiming), TUSB_DIR_OUT},
       {gs_usb_breq::GS_USB_BREQ_MODE, &device_mode, sizeof(device_mode), TUSB_DIR_OUT}}};

  bool apply_config(uint8_t request, uint16_t value) {
    switch (static_cast<gs_usb_breq>(request)) {
      case gs_usb_breq::GS_USB_BREQ_IDENTIFY:
      case gs_usb_breq::GS_USB_BREQ_SET_TERMINATION:
      case gs_usb_breq::GS_USB_BREQ_MODE:
      case gs_usb_breq::GS_USB_BREQ_BITTIMING:
      case gs_usb_breq::GS_USB_BREQ_HOST_FORMAT:
        return byte_order == 0xbeef;
      default:
        return false;
    }
  }
  // USB Device Driver task
  // This top level thread process all usb events and invoke callbacks
  inline void usb_device_task_function() {
    // init device stack on configured roothub port
    // This should be called after scheduler/kernel is started.
    // Otherwise it could cause kernel issue since USB IRQ handler does use RTOS
    // Queue API.
    tud_init(BOARD_TUD_RHPORT);

    // RTOS forever loop
    while (true) {
      // put this thread to waiting state until there is new events
      tud_task();
    }
  }

  //--------------------------------------------------------------------+
  // USB CDC
  //--------------------------------------------------------------------+
  inline void usb_can_task_function() {
    // RTOS forever loop
    while (true) {
      static gs_host_frame frame = {0};
      static auto frame_size = sizeof(frame) - sizeof(frame.timestamp_us);
      static size_t count = 0;

      while ((count = tud_vendor_available()) > 0 && !data_in_queue.is_full()) {
        count = tud_vendor_read(&frame, frame_size);
        if (count != frame_size) {
          Error_Handler();
        }
        data_in_queue.send(frame, 0);
        if ((tud_vendor_write_available()) >= frame_size) {
          tud_vendor_write(&frame, frame_size);
        } else {
          data_out_queue.send(frame, 0);
        }
      }

      while ((count = tud_vendor_write_available()) >= frame_size && data_out_queue.receive(frame, 1)) {
        tud_vendor_write(&frame, frame_size);
      }

      tud_vendor_flush();
      vTaskDelay(1);
    }
  }
};
}  // namespace unav::io::usb

extern "C" {
// Invoked when device is mounted
void tud_mount_cb(void) {}

// Invoked when device is unmounted
void tud_umount_cb(void) {}

bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, const tusb_control_request_t *request) {
  if (unav::io::usb::UsbCan::get_instance() != nullptr) {
    return unav::io::usb::UsbCan::get_instance()->control_xfer_cb(rhport, stage, request);
  }
  return false;
}
}
// // Invoked when usb bus is resumed
// void tud_resume_cb(void) {
// }

#endif /* CDC_H */
