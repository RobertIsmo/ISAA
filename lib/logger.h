#ifndef LOGGER_H
#define LOGGER_H

#include <stdbool.h>

#define LOG_BUFFER_SIZE 1024
#define ANSI_RESET "\033[0m"
#define ANSI_DEBUG "\033[0;34m"
#define ANSI_INFO "\033[0;30m"
#define ANSI_WARNING "\033[0;33m"
#define ANSI_ERROR "\033[0;31m"
#define ANSI_CRITICAL "\033[1;91m"

void isaa_log_debug(const bool toStdOutOnly, const char * topic, const char * format, ...);
void isaa_log_info(const bool toStdOutOnly, const char * topic, const char * format, ...);
void isaa_log_warning(const bool toStdOutOnly, const char * topic, const char * format, ...);
void isaa_log_error(const bool toStdOutOnly, const char * topic, const char * format, ...);
void isaa_log_critical(const bool toStdOutOnly, const char * topic, const char * format, ...);

void isaa_log_cli_help(void);
void isaa_log_daemon_help(void);

#endif // LOGGER_H
