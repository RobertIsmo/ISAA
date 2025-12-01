#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <syslog.h>
#include "logger.h"

#ifdef ISAA_DEBUG
void isaa_log_debug(const bool toStdOutOnly, const char * topic, const char * format, ...) {
	char logBuffer[LOG_BUFFER_SIZE] = {0};
	va_list args;
	va_start(args, format);
	vsprintf(logBuffer, format, args);
	
	printf("%s[%s]: DEBUG: %s%s\n", ANSI_DEBUG, topic, logBuffer, ANSI_RESET);
	if (!toStdOutOnly) {
		openlog(topic, LOG_PID, LOG_DAEMON);
		syslog(LOG_DEBUG, "DEBUG: %s", logBuffer);
		closelog();
	}
}
#else
void isaa_log_debug(const bool toStdOutOnly, const char * topic, const char * format, ...) {
	(void)toStdOutOnly;
	(void)topic;
	(void)format;
}
#endif // ISAA_DEBUG

void isaa_log_info(const bool toStdOutOnly, const char * topic, const char * format, ...) {
	char logBuffer[LOG_BUFFER_SIZE] = {0};
	va_list args;
	va_start(args, format);
	vsprintf(logBuffer, format, args);
	
	printf("%s[%s]: INFO: %s%s\n", ANSI_INFO, topic, logBuffer, ANSI_RESET);
	if (!toStdOutOnly) {
		openlog(topic, LOG_PID, LOG_DAEMON);
		syslog(LOG_INFO, "INFO: %s", logBuffer);
		closelog();
	}
}
void isaa_log_warning(const bool toStdOutOnly, const char * topic, const char * format, ...) {
	char logBuffer[LOG_BUFFER_SIZE] = {0};
	va_list args;
	va_start(args, format);
	vsprintf(logBuffer, format, args);
	
	printf("%s[%s]: WARNING: %s%s\n", ANSI_WARNING, topic, logBuffer, ANSI_RESET);
	if (!toStdOutOnly) {
		openlog(topic, LOG_PID, LOG_DAEMON);
		syslog(LOG_WARNING, "WARNING: %s", logBuffer);
		closelog();
	}
}
void isaa_log_error(const bool toStdOutOnly, const char * topic, const char * format, ...) {
	char logBuffer[LOG_BUFFER_SIZE] = {0};
	va_list args;
	va_start(args, format);
	vsprintf(logBuffer, format, args);
	
	printf("%s[%s]: ERROR: %s%s\n", ANSI_ERROR, topic, logBuffer, ANSI_RESET);
	if (!toStdOutOnly) {
		openlog(topic, LOG_PID, LOG_DAEMON);
		syslog(LOG_ERR, "ERROR: %s", logBuffer);
		closelog();
	}
}
void isaa_log_critical(const bool toStdOutOnly, const char * topic, const char * format, ...) {
	char logBuffer[LOG_BUFFER_SIZE] = {0};
	va_list args;
	va_start(args, format);
	vsprintf(logBuffer, format, args);
	
	printf("%s[%s]: CRITICAL: %s%s\n", ANSI_CRITICAL, topic, logBuffer, ANSI_RESET);
	if (!toStdOutOnly) {
		openlog(topic, LOG_PID, LOG_DAEMON);
		syslog(LOG_CRIT, "CRITICAL: %s", logBuffer);
		closelog();
	}
}

void isaa_log_cli_help(void) {
	printf("Usage: isaa [OPTIONS...] [-- COMMAND ...]\n\n");
	printf("Interior Sergeant-At-Arms is a singleton process manager. It's job is to make \nsure only one process of a certain class is running at a time. isaa is the cli \nused to run processes. isaad must be running.\n\n");
	printf("  -h, --help             To bring up this menu.\n");
	printf("  -v, --version          To get the current version..\n");
	printf("  -i, --ignore  <name>   Don't run if any process with name is running, else run.\n");
	printf("  -t, --toggle  <name>   Terminate any process running with name, else run.\n");
	printf("  -r, --replace <name>   Terminate any process running with name, then run.\n");
	printf("  --                     Everything after this will be treated as the command and arguments to run.\n");
}
void isaa_log_daemon_help(void) {
	printf("Usage: isaad [-h|-v|--help|--version]\n\n");
	printf("Interior Sergeant-At-Arms Daemon is a singleton process manager. It's job is to make sure \nonly one process of a certain class is running at a time. isaa is the cli used to run \nprocesses. Use the isaa cli program to start running processes.\n\n");
	printf("  -h, --help     To bring up this menu.\n");
	printf("  -v, --version  To get the current version..\n");
}


void isaa_log_version(void) {
#ifdef ISAA_VERSION
#define STRING(a) #a
#define DEFINITION(m) printf("Version '%s'\n", STRING(m))
	DEFINITION(ISAA_VERSION);
#else
	fprintf("No version specified.\n");
	exit(EXIT_FAILURE);
#endif // ISAA_VERSION
}
