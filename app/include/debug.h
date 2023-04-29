#include <SEGGER_RTT.h>
#ifdef ENABLE_LOGGER
#define logger_debug(args...) SEGGER_RTT_printf(0, args)
#define logger_error(args...)              \
  do {                                     \
    SEGGER_RTT_printf(0, RTT_CTRL_BG_RED); \
    SEGGER_RTT_printf(0, args);            \
    SEGGER_RTT_printf(0, RTT_CTRL_RESET);  \
  } while (false)
#else
#define debug_printf(args...)
#define logger_error(args...)
#endif