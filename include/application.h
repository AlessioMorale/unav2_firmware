#pragma once
#ifndef APPLICATION_H
#define APPLICATION_H
#include <comm/usb/cdc.h>
#include <stdint.h>

namespace unav {
enum class serial_streams_ids : size_t { COMM_SERIAL, CLI_SERIAL, COUNT };

class Application {
public:
  void setup() {
    serial_interface.setup();
  }

  Application() = default;

private:
  inline static unav::comm::usb::CDC serial_interface;
};
} // namespace unav
#endif // APPLICATION_H