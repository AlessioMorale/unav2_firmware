#ifndef CLIMODULE_H
#define CLIMODULE_H
#include "bsp/board.h"
#include "wrappers/thread.h"
#include <io/bidirectional_stream.h>
#include <cstddef>
#include <cstdint>
#include "cli_commands.h"

namespace unav::modules {
#define CLI_STACK_SIZE configMINIMAL_STACK_SIZE * 2
#define CLI_RX_BLOCKSIZE 128
#define CLI_RX_BLOCKS 2
#define CLI_TX_BLOCKSIZE 420
#define CLI_TX_BLOCKS 2

class CLIModule {
public:
  CLIModule()
      : cli_delegate{freertos::wrappers::thread_delegate::create<CLIModule, &CLIModule::cli_task_function>(*this)}, //
        cli_task{cli_delegate, ThreadPriority::Low, "cli"}, stream{"CLI"} {
  }

  void setup() {
    cli_task.create();
  }

  auto get_stream() -> io::bidirectional_stream & {
    return stream;
  }

private:
  freertos::wrappers::thread_delegate cli_delegate;
  freertos::wrappers::Thread<CLI_STACK_SIZE> cli_task;
  io::bidirectional_stream_impl<CLI_RX_BLOCKSIZE, CLI_RX_BLOCKS, CLI_TX_BLOCKSIZE, CLI_TX_BLOCKS> stream;
  cli::commands commands{stream};

  void cli_task_function() {
    commands.init();
    auto rx_stream = stream.rx_stream();
    // const size_t block_len = 64;
    io::memory_block block;
    while (true) {

      rx_stream->receive(block, Timing::MAX_DELAY);
      for (auto c : block) {
        commands.receive_char(c);
      }
      rx_stream->release(block);
      commands.process();
      vTaskDelay(1);
    }
  }
};

} // namespace unav::modules
#endif /* CLIMODULE_H */
