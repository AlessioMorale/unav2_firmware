#ifndef STREAMS_H
#define STREAMS_H

#include <cstddef>
#include <etl/bip_buffer_spsc_atomic.h>
#include <etl/memory_model.h>
#include <memory>

namespace unav::comm {

using stream = etl::ibip_buffer_spsc_atomic<uint8_t, etl::memory_model::MEMORY_MODEL_MEDIUM>;

template <const size_t SIZE>
class stream_impl : public etl::bip_buffer_spsc_atomic<uint8_t, SIZE, etl::memory_model::MEMORY_MODEL_MEDIUM> {};

struct bidirectional_stream {
  virtual auto get_rx_stream() -> stream * = 0;
  virtual auto get_tx_stream() -> stream * = 0;
};

template <const size_t RX_SIZE, const size_t TX_SIZE> struct bidirectional_stream_impl : public bidirectional_stream {
  stream_impl<RX_SIZE> rx_stream;
  stream_impl<TX_SIZE> tx_stream;

  auto get_rx_stream() -> stream * override {
    return static_cast<stream *>(&rx_stream);
  }

  auto get_tx_stream() -> stream * override {
    return &tx_stream;
  }
};

class StreamDataProvider {
public:
  virtual void link_stream(bidirectional_stream &stream, size_t index = 0) = 0;
};
} // namespace unav::comm

#endif /* STREAMS_H */
