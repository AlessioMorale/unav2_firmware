#pragma once
#ifndef CDC_H
#define CDC_H
#include <SEGGER_RTT.h>
#include <application.h>
#include <bsp/board.h>
#include <etl/algorithm.h>
#include <etl/array.h>
#include <stm32f4xx.h>
#include <stm32f4xx_hal.h>
#include <tusb.h>
#include <wrappers/queue.h>
#include <wrappers/thread.h>

#include <cstddef>
#include <string>

#include "gs_usb_private.h"
#include "tusb_config.h"

using namespace freertos::wrappers;

namespace unav::io::usb {

#define USBD_STACK_SIZE (3 * configMINIMAL_STACK_SIZE / 2) * (CFG_TUSB_DEBUG ? 2 : 1)
#define GS_STACK_SIZE configMINIMAL_STACK_SIZE
#define DATA_QUEUE_SIZE 10
#define CONTROL_QUEUE_SIZE 10
#define CAN_CLOCK_SPEED 42000000

class UsbCan {
 private:
  struct gs_control_message {
    gs_usb_breq request_type;
    const void *data;
    size_t length;
    uint8_t direction;
  };

 public:
  inline static UsbCan *instance;

  UsbCan()
      : data_in_queue("gs_usb_in"),
        data_out_queue("gs_usb_out"),
        usb_dev_delegate{thread_delegate::create<UsbCan, &UsbCan::usb_device_task_function>(*this)},
        usb_device_task{usb_dev_delegate, ThreadPriority::Highest, "usb/dev"},
        usb_can_delegate{thread_delegate::create<UsbCan, &UsbCan::usb_can_task_function>(*this)},
        usb_can_task{usb_can_delegate, ThreadPriority::High, "usb/gs_usb"} {}

  IQueue<gs_host_frame> &get_data_in_queue() { return data_in_queue; }
  IQueue<gs_host_frame> &get_data_out_queue() { return data_out_queue; }
  /// @brief setup the cdc stack
  void setup() {
    instance = this;
    // initialise the BSP
    unav::bsp::Board::instance.init_usb_hw();

    // Create a task for tinyusb device stack
    usb_device_task.create();
    // Create CDC task
    usb_can_task.create();
  }

  bool control_xfer_cb(uint8_t rhport, uint8_t stage, const tusb_control_request_t *request) {
    if (request->bmRequestType_bit.type != TUSB_REQ_TYPE_VENDOR || request->wIndex != 0) {
      return true;
    }
    SEGGER_RTT_printf(0, "control_xfer_cb %p %p %u %u\r\n", request->bRequest, request->bmRequestType_bit.type,
                      request->wIndex, request->wLength);
    auto control_message = etl::find_if(  //
        control_messages.begin(),         //
        control_messages.end(),           //
        [request](auto i) { return static_cast<uint8_t>(i.request_type) == request->bRequest; });

    if (static_cast<uint8_t>(control_message->request_type) == request->bRequest) {
      if (stage == CONTROL_STAGE_SETUP) {
        return control_message->length == request->wLength &&
               // control_message->direction == request->bmRequestType_bit.direction &&
               tud_control_xfer(rhport, request, const_cast<void *>(control_message->data), control_message->length);
      }

      if (request->bmRequestType_bit.direction == TUSB_DIR_OUT) {
        switch (stage) {
          case CONTROL_STAGE_DATA:
            return apply_config(request->bRequest);
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
  Queue<DATA_QUEUE_SIZE, gs_host_frame> data_in_queue;
  Queue<DATA_QUEUE_SIZE, gs_host_frame> data_out_queue;

  const thread_delegate usb_dev_delegate;
  Thread<USBD_STACK_SIZE> usb_device_task;
  const thread_delegate usb_can_delegate;
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

  bool apply_config(uint8_t request) {
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
    while (1) {
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
      gs_host_frame frame = {0};
      auto frame_size = sizeof(frame) - sizeof(frame.timestamp_us);

      size_t count = 0;
      while ((count = tud_vendor_available()) > 0 && !data_in_queue.is_full()) {
        (void)tud_vendor_read((void *)&frame, frame_size);
        data_in_queue.send(frame, 1);
      }
      while (!data_in_queue.is_empty() && (count = tud_vendor_write_available()) >= frame_size) {
        if (data_in_queue.receive(frame, 1)) {
          frame.echo_id = 0xFFFFFFFF;
          tud_vendor_write((void *)&frame, frame_size);
        }
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
  if (unav::io::usb::UsbCan::instance != nullptr) {
    return unav::io::usb::UsbCan::instance->control_xfer_cb(rhport, stage, request);
  }
  return false;
}
}

void tud_dfu_runtime_reboot_to_dfu_cb() {}
// // Invoked when CDC interface received data from host
// void tud_vendor_rx_cb(uint8_t itf) {
//   (void)itf;
// }

// void tud_vendor_tx_cb(uint8_t itf, uint32_t sent_bytes){
//   (void)itf;
//   (void)sent_bytes;
// }

// // Invoked when usb bus is resumed
// void tud_resume_cb(void) {
// }

#endif /* CDC_H */
