#define _POSIX_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "isaa.h"

#define ISAA_HASH_START 7

static pid_t processBuffer[ISAA_MANAGED_NAME_COUNT] = {0};

void isaa_critical(const char * message) {
	fprintf(stderr, "\033[1;31mCritical: %s\033[0m\n", message);
}

size_t index_name(const char * name) {
	size_t len = strlen(name);
	size_t sum = ISAA_HASH_START;

	for (size_t i = 0; i < len; i++) {
		sum *= name[i] * name[i] * name[i];
		sum += 1;
		sum %= ISAA_MANAGED_NAME_COUNT;
	}

	return sum;
}

bool process_exists(const size_t index) {
	pid_t child = processBuffer[index];
	if (child != 0) {
		if (kill(child, 0) != 0) {
			processBuffer[index] = 0;
			return false;
		}
		return true;
	} else {
		return false;
	}
}

int process_stop(const size_t index) {
	pid_t child = processBuffer[index];
	if (child != 0) {
		if (kill(child, SIGTERM) == 0) {
			processBuffer[index] = 0;
			for (size_t i = 0; i < ISAA_MANAGED_NAME_COUNT; i++) {
				if (processBuffer[i] == child) {
					processBuffer[i] = 0;
				}
			}
			return EXIT_SUCCESS;
		}
		isaa_critical("Failed to stop managed process.");
		return EXIT_FAILURE;
	} else {
		return EXIT_FAILURE;
	}
}

void process_manage (const IsaaProcess process) {
	for (size_t i = 0; i < process.eventCount; i++) {
		IsaaEvent event = process.events[i];
		size_t index = index_name(event.name);
		processBuffer[index] = process.pid;
	}
}

int process_start(IsaaProcess process) {
	pid_t pid;

	pid = fork();

	if (pid < 0) {
		isaa_critical("Fork failed.");
		exit(EXIT_FAILURE);
		return -1;
	}

	if (pid > 0) {
		// parent
		process.pid = pid;
		process_manage(process);
		
		return EXIT_SUCCESS;
	} else {
		// child
		umask(0);
		if (setsid() < 0) {
			// this only exits the child, not the parent.
			isaa_critical("Child failed to set SID.");
			exit(EXIT_FAILURE);
			return -1;
		}

		if (execvp(process.command, process.args)) {
			isaa_critical(strerror(errno));
			exit(EXIT_FAILURE);
			return 0;
		}
		return EXIT_FAILURE;
	}
}

int isaa_manage_process(IsaaProcess process) {
	if (process.eventCount < 1 || process.eventCount > ISAA_MAX_EVENT_COUNT) {
		return EXIT_FAILURE;
	}

	size_t
		toggleCount = 0,
		replaceCount = 0;
	size_t
		toggleIndices[ISAA_MAX_EVENT_COUNT] = {0},
		replaceIndices[ISAA_MAX_EVENT_COUNT] = {0};
	bool
		ignore = false,
		toggle = false,
		replace = false;

	for (size_t i = 0; i < process.eventCount; i++) {
		IsaaEvent event = process.events[i];
		size_t index = index_name(event.name);
		if (process_exists(index)) {
			switch (event.type) {
				case ISAA_IGNORE:
					ignore = true;
					break;
				case ISAA_TOGGLE:
					toggle = true;
					toggleIndices[toggleCount] = index;
					toggleCount++;
					break;
				case ISAA_REPLACE:
					replace = true;
					replaceIndices[replaceCount] = index;
					replaceCount++;
					break;
				default:
					isaa_critical("Unknown event type received.");
					exit(EXIT_FAILURE);
					return -1;
			}
		}
	}
	
	if (ignore) return EXIT_SUCCESS;
	if (toggle) {
		for (size_t i = 0; i < toggleCount; i++) {
			process_stop(toggleIndices[i]);
		}
		return EXIT_SUCCESS;
	}
	if (replace) {
		for (size_t i = 0; i < replaceCount; i++) {
			process_stop(replaceIndices[i]);
		}

		process_start(process);
		return EXIT_SUCCESS;
	}

	process_start(process);
	return EXIT_SUCCESS;
}

void isaa_cleanup() {
	for (size_t i = 0; i < ISAA_MANAGED_NAME_COUNT; i++) {
		pid_t child = processBuffer[i];
		if (child != 0) {
			kill(child, SIGKILL);
			for (size_t j = i; j < ISAA_MANAGED_NAME_COUNT; j++) {
				if (processBuffer[j] == child) {
					processBuffer[j] = 0;
				}
			}

		}
	}

	exit(EXIT_SUCCESS);
}

char get_user_ftok_id() {
	return getuid() % 255;
}

void isaa_start_daemon() {
	
	key_t ipcKey = ftok(ISAA_IPC_TOKEN_FILE, get_user_ftok_id());
	int queueId = msgget(ipcKey, IPC_CREAT | 0600);
	
	if (queueId == -1) {
		isaa_critical("Cannot get an queue.");
		exit(EXIT_FAILURE);
	}

	struct msgreqbuf req;
	
	while (true) {
		if (msgrcv(queueId, &req, sizeof(req), ISAA_IPC_ID, 0) == -1) {
			isaa_critical("Received an invalid message.");
			continue;
		}

		IsaaProcess process = *(IsaaProcess *)req.mtext;
		
		struct msgresbuf res = {
			.mtype = process.pid,
			.mtext = {false},
		};

		
		if (process.eventCount < 1 || process.eventCount > ISAA_MAX_EVENT_COUNT) {
			if (msgsnd(queueId, &res, sizeof(res), 0) == -1) {
				isaa_critical(strerror(errno));
				isaa_critical("Failed to send.");
			}
			continue;
		}

		size_t
			toggleCount = 0,
			replaceCount = 0;
		size_t
			toggleIndices[ISAA_MAX_EVENT_COUNT] = {0},
			replaceIndices[ISAA_MAX_EVENT_COUNT] = {0};
		bool
			ignore = false,
			toggle = false,
			replace = false;
		
		for (size_t i = 0; i < process.eventCount; i++) {
			IsaaEvent event = process.events[i];
			size_t index = index_name(event.name);
			if (process_exists(index)) {
				switch (event.type) {
				case ISAA_IGNORE:
					ignore = true;
					break;
				case ISAA_TOGGLE:
					toggle = true;
					toggleIndices[toggleCount] = index;
					toggleCount++;
					break;
				case ISAA_REPLACE:
					replace = true;
					replaceIndices[replaceCount] = index;
					replaceCount++;
					break;
				default:
					isaa_critical("Unknown event type received.");
					exit(EXIT_FAILURE);
					return;
				}
			}
		}
		
		if (ignore) {
			if (msgsnd(queueId, &res, sizeof(res), 0) == -1) {
				isaa_critical(strerror(errno));
				isaa_critical("Failed to send.");
			}
			continue;
		}
		if (toggle) {
			for (size_t i = 0; i < toggleCount; i++) {
				process_stop(toggleIndices[i]);
			}
			if (msgsnd(queueId, &res, sizeof(res), 0) == -1) {
				isaa_critical(strerror(errno));
				isaa_critical("Failed to send.");
			}
			continue;
		}
		
		res.mtext[0] = true;
		
		if (replace) {
			for (size_t i = 0; i < replaceCount; i++) {
				process_stop(replaceIndices[i]);
			}
			process_manage(process);
			if (msgsnd(queueId, &res, sizeof(res), 0) == -1) {
				isaa_critical(strerror(errno));
				isaa_critical("Failed to send.");
			}
			continue;
		}

		process_manage(process);
		if (msgsnd(queueId, &res, sizeof(res), 0) == -1) {
			isaa_critical(strerror(errno));
			isaa_critical("Failed to send.");
		}
		continue;
	}

	
}

int isaa_attempt(IsaaProcess process) {
	pid_t pid = getpid();
	process.pid = pid;
	
	struct msgreqbuf req = {
		.mtype = ISAA_IPC_ID,
	};
	memcpy(req.mtext, &process, sizeof(process));

	key_t ipcKey = ftok(ISAA_IPC_TOKEN_FILE, get_user_ftok_id());
	if (ipcKey == -1) {
		isaa_critical("Cannot get the key.");
		exit(EXIT_FAILURE);
	}
	
	int queueId = msgget(ipcKey, 0600);
	if (queueId == -1) {
		isaa_critical("Cannot get the queue.");
		exit(EXIT_FAILURE);
	}


	if(msgsnd(queueId, &req, sizeof(req), 0) == -1) {
		isaa_critical(strerror(errno));
		exit(EXIT_FAILURE);
		return EXIT_FAILURE;
	}

	printf("Waiting for res\n");
	
	struct msgresbuf res;
	if (msgrcv(queueId, &res, sizeof(res), pid, 0) == -1) {
		isaa_critical("Received an invalid message.");
		exit(EXIT_FAILURE);
	}
	
	if ((bool)res.mtext[0]) {
		if (execvp(process.command, process.args)) {
			isaa_critical(strerror(errno));
			exit(EXIT_FAILURE);
			return EXIT_FAILURE;
		}
		return EXIT_FAILURE;
	} else {
		return EXIT_SUCCESS;
	}
}
