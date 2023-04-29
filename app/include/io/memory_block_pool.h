#ifndef CIRCULAR_BUFFER_STREAM_H
#define CIRCULAR_BUFFER_STREAM_H
#include <etl/array.h>
#include <etl/fixed_sized_memory_block_allocator.h>
#include <etl/pool.h>
#include <etl/span.h>
#include <io/stream.h>

namespace unav::io {
typedef etl::span<uint8_t> memory_block;

class memory_block_pool {
public:
  virtual bool get_block(memory_block &block, size_t size) = 0;
  virtual void release(memory_block &block) = 0;
  virtual size_t size() = 0;
  virtual size_t available() = 0;

  bool empty() {
    return size() == available();
  }

  bool full() {
    return available() == 0;
  }
};

template <const size_t MAX_BLOCK_SIZE, const size_t COUNT> class memory_block_pool_impl : public memory_block_pool {
  using block_type = etl::array<uint8_t, MAX_BLOCK_SIZE>;

public:
  memory_block_pool_impl() = default;

  bool get_block(memory_block &block) override {
    auto mem_block = _memory_pool.allocate();
    ::new (mem_block) block_type();
    if (mem_block == nullptr) {
      return false;
    }
    block = io::memory_block{static_cast<memory_block::pointer>(mem_block), size};
    return true;
  }

  void release(memory_block &block) override {
    _memory_pool.release(block.data());
  }

  size_t size() {
    return COUNT;
  }

  size_t available() {
    return _memory_pool.available();
  }

private:
  etl::pool<block_type, COUNT> _memory_pool;
};
} // namespace unav::io

#endif /* CIRCULAR_BUFFER_STREAM_H */
