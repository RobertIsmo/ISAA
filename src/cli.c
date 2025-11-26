#include "../lib/isaa.h"

int main() {
	const IsaaEvent ignoreSide = {
		.type = ISAA_REPLACE,
		.name = "side"
	};
	char * command = "wofi";
	char * args[] = {
		"",
		"--show",
		"drun",
		NULL
	};
	IsaaProcess process = {
		.eventCount = 1,
		.events = {ignoreSide, {0}},
		.command = command,
		.args = args,
	};

	return isaa_attempt(process);
}
