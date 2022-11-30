#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"
#include <application.h>
#include <cassert>
#include <stm32f4xx.h>
#include <stm32f4xx_hal.h>
#include <tusb.h>

void init_usb_hw(void);

namespace unav::comm {

#define USBD_STACK_SIZE (3 * configMINIMAL_STACK_SIZE / 2) * (CFG_TUSB_DEBUG ? 2 : 1)
#define CDC_STACK_SIZE configMINIMAL_STACK_SIZE

class usb {
public:
  void setup() {
    init_usb_hw();
    // Create a task for tinyusb device stack
    (void)xTaskCreateStatic(usb_device_task, "usbd", USBD_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, usb_device_stack, &usb_device_taskdef);
    // Create CDC task
    (void)xTaskCreateStatic(cdc_task, "cdc", CDC_STACK_SIZE, NULL, configMAX_PRIORITIES - 2, cdc_stack, &cdc_taskdef);
  }
  constexpr void set_stream(serial_stream_t stream, size_t index){
    assert(index < CFG_TUD_CDC);
  }

private:
  static serial_stream_t streams[CFG_TUD_CDC];
  // static task usb device
  StackType_t usb_device_stack[USBD_STACK_SIZE];
  StaticTask_t usb_device_taskdef;

  // static task for cdc
  StackType_t cdc_stack[CDC_STACK_SIZE];
  StaticTask_t cdc_taskdef;

  static portTASK_FUNCTION_PROTO(usb_device_task, pvParameters);
  static portTASK_FUNCTION_PROTO(cdc_task, pvParameters);

  // USB Device Driver task
  // This top level thread process all usb events and invoke callbacks
  static portTASK_FUNCTION(usb_device_task, pvParameters) {
    (void)pvParameters;

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
  static void cdc_task(void *params) {
    (void)params;

    // RTOS forever loop
    while (1) {
      for (auto itf = 0; itf < CFG_TUD_CDC; itf++) {
        auto serial_stream = &streams[itf];
        static uint8_t buf[64];

        // There are data available
        while (tud_cdc_n_available(itf)) {
          // There are data available {
          auto count = tud_cdc_n_read(itf, buf, sizeof(buf));
          if (count) {
            // Todo ensure the buffer is completely sent (i.e. return == count)
            xStreamBufferSend(serial_stream->rx_stream, buf, count, 0);
          }
        }

        while (!xStreamBufferIsEmpty(serial_stream->tx_stream)) {
          auto count = xStreamBufferReceive(serial_stream->tx_stream, buf, sizeof(buf), 0);
          tud_cdc_n_write(itf, buf, count);
          tud_cdc_n_write_flush(itf);
        }
      }
    }
  }
};
} // namespace unav::comm

//--------------------------------------------------------------------+
// Forward USB interrupt events to TinyUSB IRQ Handler
//--------------------------------------------------------------------+
extern "C" static void OTG_FS_IRQHandler(void) {
  tud_int_handler(0);
}

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
