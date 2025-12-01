#ifndef PARSE_H
#define PARSE_H

#include "../lib/isaa.h"

#define ISAA_PARSE_MAX_COMMAND_ARGS 64

typedef enum {
	ISAA_PARSE_SUCCESS = 0,
	ISAA_PARSE_HELP,
	ISAA_PARSE_VERSION,
	ISAA_PARSE_BAD,
} ISAA_PARSE_CODE;

ISAA_PARSE_CODE parse_args(
				int argc,
				char ** argv,
				IsaaProcess * processPtr
	);

#endif //PARSE_H
