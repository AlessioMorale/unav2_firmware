#pragma once
#ifndef APPLICATION_H
#define APPLICATION_H
#include <bsp/board.h>
#include <debug.h>
#include <etl/array.h>
#include <io/usb/cdc.h>
#include <modules/cli/module.h>
#include <cstdint>
#include <cstring>
namespace unav {
enum class serial_streams_ids : size_t { COMM_SERIAL, CLI_SERIAL, COUNT };

class Application {
 public:
  void setup() {
    logger_init();
    logger_debug("Application.Setup()!\r\n");
    bsp::Board::instance.init_gpio();
    bsp::Board::instance.init_leds();
    auto led = bsp::Board::instance.led(bsp::BoardLeds::activity);
    led.set(true);

    serial_interface.link_stream(cli_module.get_stream(), static_cast<size_t>(serial_streams_ids::CLI_SERIAL));
    cli_module.setup();
    serial_interface.setup();
  }

  Application() = default;

private:
  inline static unav::io::usb::CDC serial_interface;
  inline static unav::modules::CLIModule cli_module;
};
}  // namespace unav
#endif  // APPLICATION_H
