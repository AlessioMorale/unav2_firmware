#include "modules/cli/cli_commands.h"
#include "modules/cli/cmd_handlers/task_list.h"
#include <memory>
namespace unav::modules::cli {

void commands::init() {
  auto config = embeddedCliDefaultConfig();
  config->maxBindingCount = 4;
  cli = embeddedCliNew(config);
  cli->appContext = static_cast<void *>(this);
  cli->writeChar = cli_write_char;
  task_list_add(cli);
  write_string("uNav debug CLI ready\r\n");
}

void commands::receive_char(char c) {
  embeddedCliReceiveChar(cli, c);
}

void commands::process() {
  embeddedCliProcess(cli);
}

void commands::get_write_buffer(io::memory_block &block, size_t len) {
  auto tx_s = stream.tx_stream();
  while (!tx_s->get_block(block, len)) {
    vTaskDelay(1);
  }
}

void commands::send(io::memory_block &block, size_t write_size) {
  auto tx_s = stream.tx_stream();
  tx_s->send(block, write_size, Timing::MAX_DELAY);
}

void commands::write_string(const std::string &str) {
  io::memory_block block;
  get_write_buffer(block, str.size());
  std::copy(str.begin(), str.end(), block.data());
  send(block);
}

} // namespace unav::modules::cli

extern "C" {
void cli_write_char(EmbeddedCli *cli, char c) {
  auto context = static_cast<unav::modules::cli::commands *>(cli->appContext);
  std::string s{c};
  context->write_string(std::string{c});
}
}
