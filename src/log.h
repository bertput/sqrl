/**
 * @file log.h
 *
 * Interface to the logging functionality.
 */

#ifndef _LOG_H_
#define _LOG_H_

enum
{
  loglevel_none,
  loglevel_info,
  loglevel_debug,
  loglevel_warn,
  loglevel_error,
  loglevel_fatal
} loglevel_enum;



int log_msg(char *format_str, ... );

int log_level(int level, char *format_str, ... );

void log_set_loglevel(int level);

int log_info(char *format_str, ...);
int log_debug(char *format_str, ...);
int log_warn(char *format_str, ...);
int log_error(char *format_str, ...);
int log_fatal(char *format_str, ...);

#endif /* _LOG_H_ */


/** @} */
