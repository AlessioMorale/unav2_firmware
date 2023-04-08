#ifndef INSTRUMENTATION_H
#define INSTRUMENTATION_H
#include <FreeRTOS.h>
#include <etl/algorithm.h>
#include <assert.h>
#include <etl/vector.h>
#include <etl/limits.h>
#include <etl/memory.h>
#include <etl/string_view.h>
#include <timing.h>

#define MAX_COUNTERS 20

namespace unav {

class PerfCounter {
public:
  etl::string_view *key;
  int32_t max;
  int32_t min;
  int32_t value;
  uint32_t last_update_us;

private:
  tickstime_t partial_timer;

public:
  PerfCounter(etl::string_view& counter_key) {
    key = &counter_key;
    max = etl::numeric_limits<int32_t>::min() + 1;
    min = etl::numeric_limits<int32_t>::max() - 1;
    last_update_us = Timing::get_us();
    partial_timer = 0;
  }

  /**
   * Update a counter with a new value
   * @param newValue the updated value.
   */

  inline void set(int32_t new_value) {
    value = new_value;
    update_stats();
  }

  /**
   * Used to determine the time duration of a code block, mark the begin of the
   * block. @see Instrumentation_TimeEnd
   */
  inline void timed_start() {
    partial_timer = Timing::get_ticks();
  }

  /**
   * Used to determine the time duration of a code block, mark the end of the
   * block. @see timed_start
   */
  inline void timed_end() {
    set((int32_t)(Timing::get_us_since(partial_timer)));
  }

  /**
   * Used to determine the average period between each call to the function
   */
  inline void track_period() {
    if (partial_timer != 0) {
      uint32_t period = Timing::get_us_since(partial_timer);
      partial_timer = Timing::get_ticks();
      set((value * 15 + period) / 16);
    }
  }

    /**
     * Increment a counter with a value
     * @param increment the value to increment counter with.
     */
    inline void increment(int32_t increment) {
      set(value + increment);
    }

  private:
    inline void update_stats() {
      max--;
      if (value > max) {
        max = value;
      }

      min++;
      if (value < min) {
        min = value;
      }
      last_update_us = Timing::get_us();
    }
};

class Instrumentation {
public:
  etl::unique_ptr<Instrumentation> instance;
  /**
   * Initialize the Instrumentation infrastructure
   */

  Instrumentation() {
  }

private:
  etl::vector<PerfCounter, MAX_COUNTERS> counters;
  /**
   * search a counter index by its unique Id
   * @param id the unique id to assign to the counter.
   * If a counter with the same id exists, the previous instance is returned
   * @return the counter handle to be used to manage its content
   */
  PerfCounter *search(etl::string_view &key) {
    auto is_key = [key](PerfCounter &p) { return *(p.key) == key; };
    auto ret = std::find_if(counters.begin(), counters.end(), is_key);
    if (*(ret->key) == key) {
      return ret;
    }
    return nullptr;
  }
  /**
   * Create a new counter.
   * @param id the unique id to assign to the counter
   * @return the counter to be used to manage its content
   */
  PerfCounter &create(etl::string_view &key) {
    assert(!counters.full());
    return counters.emplace_back(key);
  }
};
} // namespace unav
#endif /* INSTRUMENTATION_H */
