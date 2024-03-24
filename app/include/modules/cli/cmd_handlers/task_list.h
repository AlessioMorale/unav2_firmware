#pragma once
#ifndef TASK_LIST_H
#define TASK_LIST_H
#include <FreeRTOS.h>
#include <task.h>
#include <modules/cli/cli_commands.h>
extern "C" {
const char *const str_ps_header =
    "Task          State  Priority  Stack	#\r\n************************************************\r\n";
const char *const str_perf_header =
    "Task          State  Priority  Stack	#\r\n************************************************\r\n";

char *to_char_buffer(unav::io::memory_block &buffer) {
  return static_cast<char *>(static_cast<void *>(buffer.data()));
}

void onps(EmbeddedCli *cli, char *args, void *context) {
  auto cmd = static_cast<unav::modules::cli::commands *>(cli->appContext);
  unav::io::memory_block block;
  cmd->get_write_buffer(block, 400);

  auto char_buffer = to_char_buffer(block);

  strcpy(char_buffer, str_ps_header);

  vTaskList(char_buffer + strlen(str_ps_header));

  auto len = strlen(char_buffer);
  cmd->send(block, len);
}

void onperf(EmbeddedCli *cli, char *args, void *context) {
  auto cmd = static_cast<unav::modules::cli::commands *>(cli->appContext);
  unav::io::memory_block block;
  cmd->get_write_buffer(block, 400);

  auto char_buffer = to_char_buffer(block);

  strcpy(char_buffer, str_perf_header);
  vTaskGetRunTimeStats(char_buffer + strlen(str_perf_header));
  auto len = strlen(char_buffer);
  cmd->send(block, len);
}

void task_list_add(EmbeddedCli *cli) {
  embeddedCliAddBinding(cli, {"ps", "Get process list", false, nullptr, onps});
  embeddedCliAddBinding(cli, {"perf", "Get processes timings", false, nullptr, onperf});
}

// static const char *const taskListHeader = "Tasks list";

// /* This function implements the behaviour of a command, so must have the correct
// prototype. */
// static BaseType_t prvTaskStatsCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
//   memcpy(pcWriteBuffer, pcTaskListHeader, strlen(pcTaskListHeader));
//   vTaskList(pcWriteBuffer + strlen(pcTaskListHeader));

//   return pdFALSE;
// }

// static const CLI_Command_Definition_t xPSCommand = {"ps", "ps: Generate tasks list", prvTaskStatsCommand, 0};
}
#endif /* TASK_LIST_H */
