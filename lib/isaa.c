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
#include "logger.h"

#define ISAA_HASH_START 7

static pid_t processBuffer[ISAA_MANAGED_NAME_COUNT] = {0};

size_t index_name(const char * name) {
	size_t len = strlen(name);
	size_t sum = ISAA_HASH_START;

	for (size_t i = 0; i < len; i++) {
		sum *= name[i] * name[i] * name[i];
		sum += 1;
		sum %= ISAA_MANAGED_NAME_COUNT;
	}

	isaa_log_debug(false, "isaa daemon", "Index %zu from name %s.", sum, name);

	return sum;
}

bool process_exists(const size_t index) {
	pid_t child = processBuffer[index];
	if (child != 0) {
		if (kill(child, 0) != 0) {
			processBuffer[index] = 0;
			isaa_log_debug(false, "isaa daemon", "Process at index %zu exited elsewhere.", index);
			return false;
		}
		isaa_log_debug(false, "isaa daemon", "Process at index %zu exists already.", index);
		return true;
	} else {
		return false;
	}
}

int process_stop(const size_t index) {
	pid_t child = processBuffer[index];
	isaa_log_debug(false, "isaa daemon", "Stopping process with pid %d...", child);
	if (child != 0) {
		if (kill(-child, SIGTERM) == 0) {
			processBuffer[index] = 0;
			for (size_t i = 0; i < ISAA_MANAGED_NAME_COUNT; i++) {
				if (processBuffer[i] == child) {
					processBuffer[i] = 0;
				}
			}
			isaa_log_debug(false, "isaa daemon", "Cleared all process with pid %d.", child);
			return EXIT_SUCCESS;
		}
		isaa_log_critical(false, "isaa daemon", "%s:%d", __FILE__, __LINE__);
		isaa_log_critical(false, "isaa daemon", "Failed to stop managed process.");
		return EXIT_FAILURE;
	} else {
		return EXIT_FAILURE;
	}
}

void process_manage (const IsaaProcess process) {
	isaa_log_info(false, "isaa daemon", "Managing process with pid %d.", process.pid);
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
		isaa_log_critical(false, "isaa daemon", "%s:%d", __FILE__, __LINE__);
		isaa_log_critical(false, "isaa daemon", "Fork failed.");
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
			isaa_log_critical(false, "isaa daemon", "%s:%d", __FILE__, __LINE__);
			isaa_log_critical(false, "isaa daemon", "Child failed to set SID.");
			exit(EXIT_FAILURE);
			return -1;
		}

		if (execvp(process.command, process.args)) {
			isaa_log_critical(false, "isaa daemon", "%s:%d", __FILE__, __LINE__);
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
					isaa_log_critical(false, "isaa daemon", "%s:%d", __FILE__, __LINE__);
					isaa_log_critical(false, "isaa daemon", "Unknown event type received.");
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
	isaa_log_warning(false, "isaa daemon", "Running cleanup...");
	for (size_t i = 0; i < ISAA_MANAGED_NAME_COUNT; i++) {
		pid_t child = processBuffer[i];
		if (child != 0) {
			kill(-child, SIGKILL);
			for (size_t j = i; j < ISAA_MANAGED_NAME_COUNT; j++) {
				if (processBuffer[j] == child) {
					processBuffer[j] = 0;
				}
			}

		}
	}
}

char get_user_ftok_id() {
	return getuid() % 255;
}

void rm_msgq() {
	isaa_log_warning(false, "isaa daemon", "Destroying old queue...");
	key_t ipcKey = ftok(ISAA_IPC_TOKEN_FILE, get_user_ftok_id());
	int queueId = msgget(ipcKey, 0600);

	msgctl(queueId, IPC_RMID, NULL);
}

void guarentee_tmpf() {
	if (fopen(ISAA_IPC_TOKEN_FILE, "w") == NULL) {
		isaa_log_critical(false, "isaa daemon", "Cannot create token file.");
		exit(EXIT_FAILURE);
	}
}

void exit_daemon() {
	rm_msgq();
	isaa_cleanup();
}

void exit_cli() {
	isaa_log_debug(false, "isaa cli", "Exiting...");
}

void isaa_start_daemon() {
	isaa_log_info(false, "isaa daemon", "Starting ISAA Daemon...");

	guarentee_tmpf();
	
	key_t ipcKey = ftok(ISAA_IPC_TOKEN_FILE, get_user_ftok_id());
	int queueId = msgget(ipcKey, IPC_CREAT | IPC_EXCL | 0600);

	atexit(exit_daemon);
	
	if (queueId == -1) {
		isaa_log_critical(false, "isaa daemon", "%s:%d", __FILE__, __LINE__);
		isaa_log_critical(false, "isaa daemon", "Cannot get the queue.");
		exit(EXIT_FAILURE);
	}

	struct msgreqbuf req;
	
	while (true) {
		guarentee_tmpf();
		
		if (msgrcv(queueId, &req, sizeof(req), ISAA_IPC_ID, 0) == -1) {
			isaa_log_critical(false, "isaa daemon", "%s:%d", __FILE__, __LINE__);
			isaa_log_critical(false, "isaa daemon", "Received an invalid message.");
			exit(EXIT_FAILURE);
			continue;
		}

		IsaaProcess process = *(IsaaProcess *)req.mtext;

		isaa_log_info(false, "isaa daemon", "Received process attempt from pid %d with %zu event(s).", process.pid, process.eventCount);
		
		struct msgresbuf res = {
			.mtype = process.pid,
			.mtext = {false},
		};

		
		if (process.eventCount < 1 || process.eventCount > ISAA_MAX_EVENT_COUNT) {
			isaa_log_error(false, "isaa daemon", "Received no events, so telling pid %d to not run.", process.pid);
			if (msgsnd(queueId, &res, sizeof(res), 0) == -1) {
				isaa_log_critical(false, "isaa daemon", "%s:%d", __FILE__, __LINE__);
				isaa_log_critical(false, "isaa daemon", "%s:%d", __FILE__, __LINE__);
				isaa_log_critical(false, "isaa daemon", "Failed to send.");				
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
					isaa_log_critical(false, "isaa daemon", "%s:%d", __FILE__, __LINE__);
					isaa_log_critical(false, "isaa daemon", "Unknown event type received.");
					exit(EXIT_FAILURE);
					return;
				}
			}
		}
		
		if (ignore) {
			isaa_log_info(false, "isaa daemon", "Ignored. Telling pid %d to not run.", process.pid);
			if (msgsnd(queueId, &res, sizeof(res), 0) == -1) {
				isaa_log_critical(false, "isaa daemon", "%s:%d", __FILE__, __LINE__);
				isaa_log_critical(false, "isaa daemon", "%s:%d", __FILE__, __LINE__);
				isaa_log_critical(false, "isaa daemon", "Failed to send.");
			}
			continue;
		}
		if (toggle) {
			isaa_log_info(false, "isaa daemon", "Toggled. Telling pid %d to not run.", process.pid);
			if (msgsnd(queueId, &res, sizeof(res), 0) == -1) {
				isaa_log_critical(false, "isaa daemon", "%s:%d", __FILE__, __LINE__);
				isaa_log_critical(false, "isaa daemon", "%s:%d", __FILE__, __LINE__);
				isaa_log_critical(false, "isaa daemon", "Failed to send.");
			}
			for (size_t i = 0; i < toggleCount; i++) {
				process_stop(toggleIndices[i]);
			}
			continue;
		}
		
		res.mtext[0] = true;
		
		if (replace) {
			for (size_t i = 0; i < replaceCount; i++) {
				process_stop(replaceIndices[i]);
			}
			process_manage(process);
			isaa_log_info(false, "isaa daemon", "Replacing... Telling pid %d to run.", process.pid);
			if (msgsnd(queueId, &res, sizeof(res), 0) == -1) {
				isaa_log_critical(false, "isaa daemon", "%s:%d", __FILE__, __LINE__);
				isaa_log_critical(false, "isaa daemon", "%s:%d", __FILE__, __LINE__);
				isaa_log_critical(false, "isaa daemon", "Failed to send.");
			}
			continue;
		}

		process_manage(process);
		isaa_log_info(false, "isaa daemon", "No conflict. Telling pid %d to run.", process.pid);
		if (msgsnd(queueId, &res, sizeof(res), 0) == -1) {
			isaa_log_critical(false, "isaa daemon", "%s:%d", __FILE__, __LINE__);
			isaa_log_critical(false, "isaa daemon", "%s:%d", __FILE__, __LINE__);
			isaa_log_critical(false, "isaa daemon", "Failed to send.");
		}
		continue;
	}

	
}

int isaa_attempt(IsaaProcess process) {
	pid_t pid = getpid();
	process.pid = pid;

	isaa_log_debug(false, "isaa cli", "Attempting to run command %s. My pid is %d.", process.command, process.pid);

	if (setsid() >= 0) {
		isaa_log_warning(false, "isaa cli", "Setting SID. I am now a group leader.");
	} 

	atexit(exit_cli);
	
	struct msgreqbuf req = {
		.mtype = ISAA_IPC_ID,
	};
	memcpy(req.mtext, &process, sizeof(process));

	key_t ipcKey = ftok(ISAA_IPC_TOKEN_FILE, get_user_ftok_id());
	if (ipcKey == -1) {
		isaa_log_critical(false, "isaa cli", "%s:%d", __FILE__, __LINE__);
		isaa_log_critical(false, "isaa cli", "Cannot get the key.");
		exit(EXIT_FAILURE);
	}
	
	int queueId = msgget(ipcKey, 0600);
	if (queueId == -1) {
		isaa_log_critical(false, "isaa cli", "%s:%d", __FILE__, __LINE__);
		isaa_log_critical(false, "isaa cli", "Cannot get the queue.");
		exit(EXIT_FAILURE);
	}


	isaa_log_debug(false, "isaa cli", "Sending run request to daemon.");
	if(msgsnd(queueId, &req, sizeof(req), 0) == -1) {
		isaa_log_critical(false, "isaa cli", "%s:%d", __FILE__, __LINE__);
		isaa_log_critical(false, "isaa cli", strerror(errno));
		exit(EXIT_FAILURE);
		return EXIT_FAILURE;
	}

	struct msgresbuf res;
	if (msgrcv(queueId, &res, sizeof(res), pid, 0) == -1) {
		isaa_log_critical(false, "isaa cli", "%s:%d", __FILE__, __LINE__);
		isaa_log_critical(false, "isaa cli", "Received an invalid message.");
		exit(EXIT_FAILURE);
	}

	isaa_log_debug(false, "isaa cli", "Received a response from the daemon.");
	
	if ((bool)res.mtext[0]) {
		isaa_log_debug(false, "isaa cli", "Response says I can run. Running...");
		
		if (execvp(process.command, process.args)) {
			isaa_log_critical(false, "isaa cli", "%s:%d", __FILE__, __LINE__);
			isaa_log_critical(false, "isaa cli", strerror(errno));
			exit(EXIT_FAILURE);
			return EXIT_FAILURE;
		}
		return EXIT_FAILURE;
	} else {
		isaa_log_debug(false, "isaa cli", "Response says I cannot run. Exiting...");
		return EXIT_SUCCESS;
	}
}
