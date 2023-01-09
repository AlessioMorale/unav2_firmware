#pragma once
#ifndef CDC_H
#define CDC_H
#include "FreeRTOS.h"
#include "tusb_config.h"
#include <application.h>
#include <array>
#include <comm/streams.h>
#include <comm/usb/board_bsp.h>
#include <cstddef>
#include <etl/algorithm.h>
#include <stm32f4xx.h>
#include <stm32f4xx_hal.h>
#include <string>
#include <tusb.h>
#include <wrappers/staticalloc/thread.h>

using namespace freertos::wrappers;
using namespace freertos::wrappers::staticalloc;

namespace unav::comm::usb {

#define USBD_STACK_SIZE (3 * configMINIMAL_STACK_SIZE / 2) * (CFG_TUSB_DEBUG ? 2 : 1)
#define CDC_STACK_SIZE configMINIMAL_STACK_SIZE

class CDC : public StreamProvider {

public:
  CDC()
      : usb_dev_delegate{etl::delegate<void()>::create<CDC, &CDC::usb_device_task_function>(*this)}, usb_device_task{usb_dev_delegate, TaskPriority::Highest,
                                                                                                                     "usb_dev"},
        usb_cdc_delegate{etl::delegate<void()>::create<CDC, &CDC::cdc_task_function>(*this)}, usb_cdc_task{usb_cdc_delegate, TaskPriority::High, "usb_cdc"} {
  }

  void setup() {
    // initialise the BSP
    init_usb_hw();

    // Create a task for tinyusb device stack
    usb_device_task.Create();
    // Create CDC task
    usb_cdc_task.Create();
  }

  void link_stream(bidirectional_stream_t &stream, size_t index) override {
    // assert(index < CFG_TUD_CDC);
    streams[index] = &stream;
  }

private:
  const etl::delegate<void()> usb_dev_delegate;
  Thread<USBD_STACK_SIZE> usb_device_task;
  const etl::delegate<void()> usb_cdc_delegate;
  Thread<CDC_STACK_SIZE> usb_cdc_task;

  std::array<bidirectional_stream_t *, CFG_TUD_CDC> streams = {nullptr};

  // USB Device Driver task
  // This top level thread process all usb events and invoke callbacks
  inline void usb_device_task_function() {
    // init device stack on configured roothub port
    // This should be called after scheduler/kernel is started.
    // Otherwise it could cause kernel issue since USB IRQ handler does use RTOS queue API.
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
  inline void cdc_task_function() {
    const size_t block_len = 64;
    // RTOS forever loop
    while (true) {
      for (auto itf = 0; itf < CFG_TUD_CDC; itf++) {
        auto serial_stream = this->streams[itf];

        // There are data available
        size_t count = 0;
        while (serial_stream && serial_stream->rx_stream && !(serial_stream->rx_stream->available() < block_len) && (count = tud_cdc_n_available(itf)) != 0) {
          // There are data available
          auto write_buffer = serial_stream->rx_stream->write_reserve(count);
          (void)tud_cdc_n_read(itf, write_buffer.data(), write_buffer.size_bytes());
          serial_stream->rx_stream->write_commit(write_buffer);
        }

        while (serial_stream && serial_stream->tx_stream && !serial_stream->tx_stream->empty()) {
          auto read_buffer = serial_stream->tx_stream->read_reserve(block_len);
          tud_cdc_n_write(itf, read_buffer.data(), read_buffer.size_bytes());
          serial_stream->tx_stream->read_commit(read_buffer);
          tud_cdc_n_write_flush(itf);
        }
      }
    }
  }
};
} // namespace unav::comm::usb

// // Invoked when device is mounted
// void tud_mount_cb(void) {
// }

// // Invoked when device is unmounted
// void tud_umount_cb(void) {
// }

// // Invoked when cdc when line state changed e.g connected/disconnected
// void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
//   (void)itf;
//   (void)rts;
// }

// // Invoked when CDC interface received data from host
// void tud_cdc_rx_cb(uint8_t itf) {
//   (void)itf;
// }

// // Invoked when usb bus is suspended
// // remote_wakeup_en : if host allow us  to perform remote wakeup
// // Within 7ms, device must draw an average of current less than 2.5 mA from bus
// void tud_suspend_cb(bool remote_wakeup_en) {
//   (void)remote_wakeup_en;
// }

// // Invoked when usb bus is resumed
// void tud_resume_cb(void) {
// }

#endif /* CDC_H */
