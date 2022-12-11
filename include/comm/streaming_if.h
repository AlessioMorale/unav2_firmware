#include "FreeRTOS.h"
#include <stream_buffer.h>

namespace unav::comm {

typedef struct {
  StreamBufferHandle_t rx_stream;
  StreamBufferHandle_t tx_stream;
} serial_stream_t;

class streaming_if {
public:
  virtual void setup();
  virtual void link_stream(serial_stream_t stream, size_t index);
};
} // namespace unav::comm