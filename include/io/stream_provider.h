#ifndef STREAM_PROVIDER_H
#define STREAM_PROVIDER_H
#include "stream.h"
#include "bidirectional_stream.h"

namespace unav::io {

class stream_data_provider {
public:
  virtual void link_stream(stream &stream, size_t index = 0) = 0;
};

class bidirectional_stream_data_provider {
public:
  virtual void link_stream(bidirectional_stream &stream, size_t index = 0) = 0;
};
} // namespace unav::io
#endif /* STREAM_PROVIDER_H */
