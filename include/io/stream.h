#ifndef STREAM_H
#define STREAM_H

#include <cstddef>
#include <cstdint>
#include <etl/array_view.h>
#include <etl/fixed_sized_memory_block_allocator.h>
#include <etl/memory_model.h>
#include <etl/span.h>
#include <memory>
#include <wrappers/queue.h>
namespace unav::io {

typedef etl::span<uint8_t> memory_block;

struct block_pointer {
  uint8_t *data;
  size_t size;
};

class stream {
public:
  static const size_t DEFAULT_ALIGNMENT = sizeof(void *);
  virtual bool get_block(memory_block &block, size_t size) = 0;
  virtual bool send(memory_block &block, uint32_t wait_us) = 0;
  virtual bool send(memory_block &block, size_t resized_len, uint32_t wait_us) = 0;
  virtual bool receive(memory_block &block, uint32_t wait_us) = 0;
  virtual void release(memory_block &block) = 0;
  virtual bool empty() = 0;
  virtual bool full() = 0;
  };

// using stream = etl::ibip_buffer_spsc_atomic<uint8_t, etl::memory_model::MEMORY_MODEL_MEDIUM>;

template <const size_t MAX_BLOCK_SIZE, const size_t COUNT> class stream_impl : public stream {
public:
  stream_impl(const std::string &name) : name{name}, _message_queue{name} {};
  bool get_block(memory_block &block, size_t size) override {
    auto mem_block = _memory_pool.allocate(size, DEFAULT_ALIGNMENT);
    if (mem_block == nullptr) {
      return false;
    }
    block = io::memory_block{static_cast<memory_block::pointer>(mem_block), size};
    return true;
  }

  void release(memory_block &block) override {
    _memory_pool.release(block.data());
  }

  bool send(memory_block &block, uint32_t wait_us) override {
    block_pointer pointer{block.data(), block.size_bytes()};
    return _message_queue.send(pointer, wait_us);
  }

  bool send(memory_block &block, size_t resized_len, uint32_t wait_us) override {
    if (resized_len < block.size_bytes()) {
      memory_block resized_block = memory_block{block.begin(), block.begin() + resized_len};
      return send(resized_block, Timing::MAX_DELAY);
    }
    return send(block, Timing::MAX_DELAY);
  }

  bool empty() {
    return _message_queue.messages_waiting() == 0;
  }
  bool full() {
    return _message_queue.space_available() == 0;
  }
  bool receive(memory_block &block, uint32_t wait_us) override {
    block_pointer pointer;

    if (!_message_queue.receive(pointer, wait_us)) {
      return false;
    }

    block = {pointer.data, pointer.size};
    return true;
  }
  const std::string name;

private:
  freertos::wrappers::queue<COUNT, block_pointer> _message_queue;
  etl::fixed_sized_memory_block_allocator<MAX_BLOCK_SIZE, DEFAULT_ALIGNMENT, COUNT> _memory_pool;
};

} // namespace unav::io

#endif /* STREAM_H */
