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

etl::span<uint8_t> commands::get_write_buffer(size_t len) {
  auto tx_s = stream.get_tx_stream();
  return tx_s->write_reserve(len);
}

void commands::commit_write_buffer(etl::span<uint8_t> buffer, size_t write_size) {
  auto tx_s = stream.get_tx_stream();
  if (write_size != SIZE_MAX) {
    tx_s->write_commit(etl::span<uint8_t>(buffer.begin(), buffer.begin() + write_size));
  } else {
    tx_s->write_commit(buffer);
  }
}
void commands::write_string(const std::string &str) {
  auto buffer = get_write_buffer(str.size());
  std::copy(str.begin(), str.end(), buffer.begin());
  commit_write_buffer(buffer);
}

} // namespace unav::modules::cli

extern "C" {
void cli_write_char(EmbeddedCli *cli, char c) {
  auto context = static_cast<unav::modules::cli::commands *>(cli->appContext);
  auto write_buffer = context->get_write_buffer(1);
  write_buffer[0] = c;
  context->commit_write_buffer(write_buffer);
}
}
