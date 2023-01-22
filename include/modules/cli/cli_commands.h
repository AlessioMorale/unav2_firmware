#pragma once
#ifndef CLI_COMMANDS_H
#define CLI_COMMANDS_H
#include "comm/streams.h"
#include <embedded_cli.h>

extern "C" {
void cli_write_char(EmbeddedCli *cli, char c);
}

namespace unav::modules::cli {
class commands {
public:
  commands(comm::bidirectional_stream &stream) : stream{stream} {};

  void init();
  void receive_char(char c);
  void process();

  etl::span<uint8_t> get_write_buffer(size_t len);
  void commit_write_buffer(etl::span<uint8_t> buffer, size_t trim_size = SIZE_MAX);
  void write_string(const std::string &str);

  comm::bidirectional_stream &stream;

private:
  // const std::string cmd_welcome_message{"uNav CLI.rn"};
  EmbeddedCli *cli{nullptr};
};
} // namespace unav::modules::cli

#endif /* CLI_COMMANDS_H */
