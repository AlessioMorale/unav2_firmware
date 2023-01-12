#ifndef CLIMODULE_H
#define CLIMODULE_H
#include "comm/streams.h"
#include "wrappers/staticalloc/thread.h"
#include <cstddef>
#include <cstdint>

namespace unav::modules {
#define CLI_STACK_SIZE configMINIMAL_STACK_SIZE
#define CLI_RX_BUFFER 128
#define CLI_TX_BUFFER 128

class CLIModule {
public:
  CLIModule()
      : cli_delegate{freertos::wrappers::thread_delegate::create<CLIModule, &CLIModule::cli_task_function>(*this)}, //
        cli_task{cli_delegate, TaskPriority::Low, "cli"} {
  }

  void setup() {
    cli_task.Create();
  }

  auto get_stream() -> comm::bidirectional_stream & {
    return stream;
  }

private:
  freertos::wrappers::thread_delegate cli_delegate;
  freertos::wrappers::staticalloc::Thread<CLI_STACK_SIZE> cli_task;
  comm::bidirectional_stream_impl<CLI_RX_BUFFER, CLI_TX_BUFFER> stream;

  void cli_task_function() {
  }
};

} // namespace unav::modules
#endif /* CLIMODULE_H */
