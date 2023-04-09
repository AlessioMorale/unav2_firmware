#pragma once
#ifndef QUEUE_W_H
#define QUEUE_W_H

#include <FreeRTOS.h>
#include <queue.h>
#include <stddef.h>
#include <timing.h>

#include <string>

namespace freertos::wrappers {

template <typename T>
class IQueue {
 public:
  virtual bool send(const T &item, uint32_t wait_us) = 0;
  virtual bool send_from_isr(const T &item) = 0;
  virtual bool receive(T &item, uint32_t wait_us) = 0;
  virtual bool receive_from_isr(T &item) = 0;
  virtual size_t messages_waiting() = 0;
  virtual size_t space_available() = 0;
  virtual void reset() = 0;
  virtual bool is_empty_from_isr() = 0;
  virtual bool is_full_from_isr() = 0;

  bool is_empty() {
    auto count = messages_waiting();
    return  count == 0;
  }

  bool is_full() { return space_available() == 0; }
};

template <const size_t queue_size, typename T>
class Queue : public IQueue<T> {
 public:
  constexpr static size_t ITEM_SIZE = sizeof(T);

  Queue(const std::string &name) : Name{name} {
    _queue = xQueueCreateStatic(queue_size, sizeof(T), _queue_storage, &_queue_buf);
    vQueueAddToRegistry(_queue, Name.c_str());
  }

  virtual ~Queue() = default;
  const std::string Name;

  bool send(const T &item, uint32_t wait_us) override {
    auto ticks = unav::Timing::us_to_ticks(wait_us);
    auto ret = xQueueSend(_queue, static_cast<const void *>(&item), ticks);
    return ret == pdTRUE;
  }

  bool send_from_isr(const T &item) override {
    auto higher_priority_task_woken = pdFALSE;
    auto ret = xQueueSendFromISR(_queue, static_cast<const void *>(&item), &higher_priority_task_woken);
    portYIELD_FROM_ISR(higher_priority_task_woken);
    return ret == pdTRUE;
  }

  bool receive(T &item, uint32_t wait_us) override {
    auto ticks = unav::Timing::us_to_ticks(wait_us);
    auto ret = xQueueReceive(_queue, static_cast<void *>(&item), ticks);
    return ret == pdTRUE;
  }

  bool receive_from_isr(T &item) override {
    auto higher_priority_task_woken = pdFALSE;
    auto ret = xQueueReceiveFromISR(_queue, static_cast<void *>(&item), &higher_priority_task_woken);
    portYIELD_FROM_ISR(higher_priority_task_woken);
    return ret == pdTRUE;
  }

  size_t messages_waiting() override { return static_cast<size_t>(uxQueueMessagesWaiting(_queue)); }

  size_t space_available() override { return static_cast<size_t>(uxQueueSpacesAvailable(_queue)); }

  void reset() override { (void)xQueueReset(_queue); }

  bool is_empty_from_isr() override { return xQueueIsQueueEmptyFromISR(_queue) == pdTRUE; }

  bool is_full_from_isr() override { return xQueueIsQueueFullFromISR(_queue) == pdTRUE; }

 protected:
  uint8_t _queue_storage[queue_size * ITEM_SIZE]{0};
  StaticQueue_t _queue_buf;
  QueueHandle_t _queue;
};
}  // namespace freertos::wrappers

#endif /* QUEUE_W_H */
