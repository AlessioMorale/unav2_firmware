#ifndef CLIMODULE_H
#define CLIMODULE_H
#include "bsp/board.h"
#include "comm/streams.h"
#include "wrappers/staticalloc/thread.h"
#include <cstddef>
#include <cstdint>
#include "cli_commands.h"

namespace unav::modules {
#define CLI_STACK_SIZE configMINIMAL_STACK_SIZE * 2
#define CLI_RX_BUFFER 128
#define CLI_TX_BUFFER 512

class CLIModule {
public:
  CLIModule()
      : cli_delegate{freertos::wrappers::thread_delegate::create<CLIModule, &CLIModule::cli_task_function>(*this)}, //
        cli_task{cli_delegate, TaskPriority::Low, "cli"} {
  }

  void setup() {
    cli_task.create();
  }

  auto get_stream() -> comm::bidirectional_stream & {
    return stream;
  }

private:
  freertos::wrappers::thread_delegate cli_delegate;
  freertos::wrappers::staticalloc::Thread<CLI_STACK_SIZE> cli_task;
  comm::bidirectional_stream_impl<CLI_RX_BUFFER, CLI_TX_BUFFER> stream;
  cli::commands commands{stream};

  void cli_task_function() {
    commands.init();
    auto rx_stream = stream.get_rx_stream();
    const size_t block_len = 64;
    while (true) {

      if (!rx_stream->empty()) {
        auto read_buffer = rx_stream->read_reserve(block_len);
        for(auto c : read_buffer)
        {
          commands.receive_char(c);
        }
        rx_stream->read_commit(read_buffer);
      }
      commands.process();
      vTaskDelay(1);
    }
  }
};

} // namespace unav::modules
#endif /* CLIMODULE_H */
