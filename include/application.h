#pragma once
#ifndef APPLICATION_H
#define APPLICATION_H
#include <bsp/board.h>
#include <io/usb/gs_can/gs_usb.h>

#include <cstdint>
namespace unav {

class Application {
 public:
  void setup() {
    bsp::Board::instance.init_gpio();
    bsp::Board::instance.init_leds();
    auto led = bsp::Board::instance.led(bsp::BoardLeds::activity);
    led.set(true);
    usb_can.setup();
  }

  Application() = default;

 private:
  inline static unav::io::usb::UsbCan usb_can;
};
}  // namespace unav
#endif  // APPLICATION_H