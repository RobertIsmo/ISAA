#include <stdlib.h>
#include <stdbool.h>
#include "../lib/isaa.h"
#include "../lib/logger.h"
#include "parse.h"

int main(int argc, char ** argv) {
	IsaaProcess process;

	switch(parse_args(argc, argv, &process)) {
	case ISAA_PARSE_SUCCESS:
		break;
	case ISAA_PARSE_HELP:
		isaa_log_cli_help();
		return EXIT_SUCCESS;
	case ISAA_PARSE_BAD:
		isaa_log_error(true, "isaa cli", "bad arguments. Use -h/--help to see usage.");
		return EXIT_FAILURE;
	}

	for (size_t i = 0; i < process.eventCount; i++) {
		isaa_log_debug(
					   false,
					   "isaa cli",
					   "event %d %s",
					   process.events[i].type,
					   process.events[i].name
		);
	}

	return isaa_attempt(process);
}
