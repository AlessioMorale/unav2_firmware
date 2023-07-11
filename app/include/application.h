#pragma once
#ifndef APPLICATION_H
#define APPLICATION_H
#include <bsp/board.h>
#include <debug.h>
#include <io/usb/gs_can/gs_usb.h>
#include <opencyphal/opencyphal.h>

#include <cstdint>
namespace unav {

class Application {
 public:
  void setup() {
    logger_init();
    logger_debug("Application.Setup()!\r\n");
    bsp::Board::instance.init_gpio();
    bsp::Board::instance.init_leds();
    auto led = bsp::Board::instance.led(bsp::BoardLeds::activity);
    led.set(true);
    auto usb_can = unav::io::usb::UsbCan::get_instance();
    usb_can->setup();
    opencyphal_.setup(usb_can->get_data_out_queue(), usb_can->get_data_in_queue());
  }

  Application() = default;

 private:
  inline static unav::opencyphal::OpenCyphal opencyphal_;
};
}  // namespace unav
#endif  // APPLICATION_H