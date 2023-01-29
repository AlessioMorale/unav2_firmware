#ifndef BIDIRECTIONAL_STREAM_H
#define BIDIRECTIONAL_STREAM_H
#include "stream.h"
namespace unav::io {

struct bidirectional_stream {
  virtual auto rx_stream() -> stream * = 0;
  virtual auto tx_stream() -> stream * = 0;
};

template <const size_t RX_MAX_BLOCK_SIZE, const size_t RX_COUNT, const size_t TX_MAX_BLOCK_SIZE, const size_t TX_COUNT>
class bidirectional_stream_impl : public bidirectional_stream {
public:
  bidirectional_stream_impl(const std::string &name) : _rx_stream{name + "_rx"}, _tx_stream{name + "_tx"} {};
  
  auto rx_stream() -> stream * override {
    return &_rx_stream;
  }

  auto tx_stream() -> stream * override {
    return &_tx_stream;
  }
  const std::string name;

private:
  stream_impl<RX_MAX_BLOCK_SIZE, RX_COUNT> _rx_stream;
  stream_impl<TX_MAX_BLOCK_SIZE, TX_COUNT> _tx_stream;
};
} // namespace unav::io
#endif /* BIDIRECTIONAL_STREAM_H */
