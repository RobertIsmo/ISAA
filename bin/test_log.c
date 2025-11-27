#include <stdbool.h>
#include "../lib/logger.h"

int main() {
	isaa_log_debug(true, "isaa_test", "%s %d", "testing", 3);
	isaa_log_info(true, "isaa_test", "%s %d", "testing", 3);
	isaa_log_warning(true, "isaa_test", "%s %d", "testing", 3);
	isaa_log_error(true, "isaa_test", "%s %d", "testing", 3);
	isaa_log_critical(true, "isaa_test", "%s %d", "testing", 3);
	isaa_log_debug(false, "isaa_test", "%s %d", "testing", 3);
	isaa_log_info(false, "isaa_test", "%s %d", "testing", 3);
	isaa_log_warning(false, "isaa_test", "%s %d", "testing", 3);
	isaa_log_error(false, "isaa_test", "%s %d", "testing", 3);
	isaa_log_critical(false, "isaa_test", "%s %d", "testing", 3);
	isaa_log_cli_help();
	isaa_log_daemon_help();
	return 0;
}
