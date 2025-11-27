#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include "parse.h"

ISAA_PARSE_CODE parse_args(
				int argc,
				char ** argv,
				IsaaProcess * processPtr
	) {
	if (argc < 2) {
		return ISAA_PARSE_BAD;
	}
	
	size_t eventCount = 0;
	bool escape = false;
	char * command = NULL;
	size_t arglen = 1;
	char ** args = malloc(sizeof(char *) * ISAA_PARSE_MAX_COMMAND_ARGS);
	args[0] = "";
	for (int i = 1; i < argc; i++) {
		if (escape) {
			if (command) {
				args[arglen] = argv[i];
				args[arglen + 1] = NULL;
				arglen++;
				continue;
			} else {
				command = argv[i];
				continue;
			}
		} else {
			if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help")  == 0) {
				return ISAA_PARSE_HELP;
			}

			if (strcmp(argv[i], "--") == 0) {
				escape = true;
				continue;
			}

			if (strcmp(argv[i], "-i")  == 0 || strcmp(argv[i], "--ignore")  == 0) {
				IsaaEvent event = { ISAA_IGNORE, {0} };
				memcpy(event.name, argv[i + 1], ISAA_MAX_NAME_LENGTH);
				processPtr->events[eventCount] = event;
				processPtr->events[eventCount + 1] = (IsaaEvent){0};
				eventCount++;
				i++;
			} else if (strcmp(argv[i], "-t")  == 0 || strcmp(argv[i], "--toggle")  == 0) {
				IsaaEvent event = { ISAA_TOGGLE, {0} };
				memcpy(event.name, argv[i + 1], ISAA_MAX_NAME_LENGTH);
				processPtr->events[eventCount] = event;
				processPtr->events[eventCount + 1] = (IsaaEvent){0};
				eventCount++;
				i++;
			} else if (strcmp(argv[i], "-r")  == 0 || strcmp(argv[i], "--replace")  == 0) {
				IsaaEvent event = { ISAA_REPLACE, {0} };
				memcpy(event.name, argv[i + 1], ISAA_MAX_NAME_LENGTH);
				processPtr->events[eventCount] = event;
				processPtr->events[eventCount + 1] = (IsaaEvent){0};
				eventCount++;
				i++;
			} else {
				fprintf(stderr, "Error: unknown argument %s.\n", argv[i]);
				return ISAA_PARSE_BAD;
			}
		}
	}
	
	if (argc < 5) {
		return ISAA_PARSE_BAD;
	}
	
	processPtr->eventCount = eventCount;
	processPtr->command = command;
	processPtr->args = args;
	return ISAA_PARSE_SUCCESS;
}
