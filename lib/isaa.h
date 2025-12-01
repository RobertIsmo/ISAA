#ifndef ISAA_H
#define ISAA_H

#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>

#define ISAA_IPC_TOKEN_FILE "/tmp/isaa_f"
#define ISAA_IPC_ID 1
#define ISAA_MANAGED_NAME_COUNT 65536
#define ISAA_MAX_NAME_LENGTH 64
#define ISAA_MAX_EVENT_COUNT 32

// This ordering also represents the priority of the events.
typedef enum {
	ISAA_IGNORE = 1,
	ISAA_TOGGLE,
	ISAA_REPLACE,
} IsaaEventType;

typedef struct {
	IsaaEventType   type;
	char            name[ISAA_MAX_NAME_LENGTH];
} IsaaEvent;

typedef struct {
	size_t       eventCount;
	IsaaEvent    events[ISAA_MAX_EVENT_COUNT];
	
	pid_t              pid;
	char            *  command;
	char            ** args;
} IsaaProcess;

struct msgreqbuf {
	long mtype;
	char mtext[sizeof(IsaaProcess)];
};

struct msgresbuf {
	long mtype;
	char mtext[sizeof(bool)];
};

// Single Process Functions
// Generally you should not use this, mainly for testing.
int isaa_manage_process(IsaaProcess process);
void isaa_cleanup(void);

// Daemon Functions
void isaa_start_daemon(void);

// Child Functions
int isaa_attempt(IsaaProcess process);

#endif //ISAA_H
