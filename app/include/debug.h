#include <SEGGER_RTT.h>
#ifdef ENABLE_LOGGER
#define logger_debug(args...) SEGGER_RTT_printf(0, args)
#define logger_error(args...)              \
  do {                                     \
    SEGGER_RTT_printf(0, RTT_CTRL_BG_RED); \
    SEGGER_RTT_printf(0, args);            \
    SEGGER_RTT_printf(0, RTT_CTRL_RESET);  \
  } while (false)
#define logger_init() SEGGER_RTT_Init()
#else
#define debug_printf(args...)
#define logger_error(args...)
#define logger_init()
#endif

extern "C" void Error_Handler(void) __attribute__((noreturn));
