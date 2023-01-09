#ifndef STREAMS_H
#define STREAMS_H

#include <etl/memory_model.h>
#include <etl/bip_buffer_spsc_atomic.h>

namespace unav::comm {

struct bidirectional_stream_t {
  etl::ibip_buffer_spsc_atomic<uint8_t, etl::memory_model::MEMORY_MODEL_MEDIUM> *rx_stream;
  etl::ibip_buffer_spsc_atomic<uint8_t, etl::memory_model::MEMORY_MODEL_MEDIUM> *tx_stream;
};

class StreamProvider {
  public:
  virtual void link_stream(bidirectional_stream_t &stream, size_t index = 0) = 0;
};
} // namespace unav::comm

#endif /* STREAMS_H */
