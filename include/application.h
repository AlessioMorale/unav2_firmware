#pragma once
#ifndef APPLICATION_H
#define APPLICATION_H
#include "configuration.h"
#include <FreeRTOS.h>
#include <inttypes.h>
#include <comm/streaming_if.h>
using unav::comm::serial_stream_t;

namespace unav {
enum class serial_streams_ids : size_t { COMM_SERIAL, CLI_SERIAL, COUNT };

class Application {
public:
  static serial_stream_t &get_stream(serial_streams_ids stream_id) {
    return serial_streams[static_cast<size_t>(stream_id)];
  }

  static void Setup() {
    create_streams();
  }

private:
  inline static usb::cdc interface;
  inline static unav::comm::serial_stream_t serial_streams[static_cast<size_t>(serial_streams_ids::COUNT)];

  static void create_streams() {
    static StaticStreamBuffer_t cli_rx_streambuffer;
    static uint8_t cli_rx_buffer_storage[CLI_INPUT_BUFFER_SIZE];
    static StaticStreamBuffer_t cli_tx_streambuffer;
    static uint8_t cli_tx_buffer_storage[CLI_OUTPUT_BUFFER_SIZE];

    static StaticStreamBuffer_t comm_rx_streambuffer;
    static uint8_t comm_rx_buffer_storage[COMM_INPUT_BUFFER_SIZE];
    static StaticStreamBuffer_t comm_tx_streambuffer;
    static uint8_t comm_tx_buffer_storage[COMM_OUTPUT_BUFFER_SIZE];

    auto stream = &serial_streams[static_cast<size_t>(serial_streams_ids::CLI_SERIAL)];
    stream->rx_stream = xStreamBufferCreateStatic(sizeof(cli_rx_buffer_storage), 1, cli_rx_buffer_storage, &cli_rx_streambuffer);
    stream->tx_stream = xStreamBufferCreateStatic(sizeof(cli_tx_buffer_storage), 1, cli_tx_buffer_storage, &cli_tx_streambuffer);

    stream = &serial_streams[static_cast<size_t>(serial_streams_ids::COMM_SERIAL)];
    stream->rx_stream = xStreamBufferCreateStatic(sizeof(comm_rx_buffer_storage), 1, comm_rx_buffer_storage, &comm_rx_streambuffer);
    stream->tx_stream = xStreamBufferCreateStatic(sizeof(comm_tx_buffer_storage), 1, comm_tx_buffer_storage, &comm_tx_streambuffer);
  }

  Application(){};
  };
  }    // namespace unav
#endif // APPLICATION_H