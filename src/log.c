#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "log.h"

static int loglevel;

void log_set_loglevel(int level)
{
  loglevel = level;
}

void log_print_loglevel(int level)
{
  switch (level)
  {
    case loglevel_info  : printf("LOG_INFO: "); break;
    case loglevel_debug : printf("LOG_DEBUG: "); break;
    case loglevel_warn  : printf("LOG_WARN: "); break;
    case loglevel_error : printf("LOG_ERROR: "); break;
    case loglevel_fatal :
    default             : printf("LOG_FATAL: "); break;
  }
}

int log_print_common_data(void)
{
  int retval;
  struct tm *tm;
  time_t     t;

  char time_str[27];

  t = time(NULL);
  tm = localtime(&t);

  /* Do not print \n here; the message immediately follows the time.
   */
  strftime(time_str, 26, "%Y/%m/%d %I:%M:%S %p", tm);
  retval = printf("[%s] : ", time_str);

  return retval;
}

int log_msg_v(char *format_str, va_list argp)
{
	return vprintf(format_str, argp);
}

int log_msg(char *format_str, ... )
{
  int retval;
  va_list marker;

  log_print_common_data();

  va_start(marker, format_str);     /* Initialize variable arguments. */
  retval = log_msg_v(format_str, marker);
  va_end(marker);                   /* Reset variable arguments.      */

  return retval;
}

int log_level_v(int level, char *format_str, va_list argp)
{
  int retval;

  if (level <= loglevel || level >= loglevel_error) // always print error and fatal.
  {
    log_print_loglevel(level);
    log_print_common_data();

    retval = log_msg_v(format_str, argp);
  }

  return retval;
}

int log_level(int level, char *format_str, ... )
{
  int retval;
  va_list marker;

  va_start(marker, format_str);     /* Initialize variable arguments. */
  retval = log_level_v(level, format_str, marker);
  va_end(marker);                   /* Reset variable arguments.      */

  return retval;
}

int log_info(char *format_str, ...)
{
  int retval;
  va_list marker;

  va_start(marker, format_str);     /* Initialize variable arguments. */
  retval = log_level_v(loglevel_info, format_str, marker);
  va_end(marker);                   /* Reset variable arguments.      */

  return retval;
}

int log_debug(char *format_str, ...)
{
  int retval;
  va_list marker;

  va_start(marker, format_str);     /* Initialize variable arguments. */
  retval = log_level_v(loglevel_debug, format_str, marker);
  va_end(marker);                   /* Reset variable arguments.      */

  return retval;
}

int log_warn(char *format_str, ...)
{
  int retval;
  va_list marker;

  va_start(marker, format_str);     /* Initialize variable arguments. */
  retval = log_level_v(loglevel_warn, format_str, marker);
  va_end(marker);                   /* Reset variable arguments.      */

  return retval;
}

int log_error(char *format_str, ...)
{
  int retval;
  va_list marker;

  va_start(marker, format_str);     /* Initialize variable arguments. */
  retval = log_level_v(loglevel_error, format_str, marker);
  va_end(marker);                   /* Reset variable arguments.      */

  return retval;
}

int log_fatal(char *format_str, ...)
{
  int retval;
  va_list marker;

  va_start(marker, format_str);     /* Initialize variable arguments. */
  retval = log_level_v(loglevel_fatal, format_str, marker);
  va_end(marker);                   /* Reset variable arguments.      */

  return retval;
}
