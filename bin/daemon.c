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
		isaa_log_daemon_help();
		return EXIT_SUCCESS;
	case ISAA_PARSE_BAD:
		break;
	}
	
	isaa_start_daemon();
}
