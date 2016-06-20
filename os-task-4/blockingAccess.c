#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define MAX_MODE_LENGTH 6
#define MAX_USERS 1000
#define MAX_USER_LENGTH 50
#define MAX_PASSWORD_LENGTH 50

char * getLockName(char * fileName) {
	char * lockName = malloc(strlen(fileName) + 5);

	strcpy(lockName, fileName);
	strcat(lockName, ".lck\0");

	return lockName;
}

int isFileBlocked(char * fileName) {
	return access(getLockName(fileName), 0) != -1;
}

int blockFile(char * fileName, char * mode) {
	if (isFileBlocked(fileName)) {
		return -1;
	}
	
	FILE * lockPointer = fopen(getLockName(fileName), "w");

	fprintf(lockPointer, "%d:%s", getpid(), mode);
	fclose(lockPointer);

	return 0;
}

int clearFileBlock(char * fileName) {
	if (!isFileBlocked(fileName)) {
		return 0;
	}

	char * fileLockName = getLockName(fileName);
	FILE * lockPointer = fopen(fileLockName, "r");
	int pid;
	char mode[MAX_MODE_LENGTH];

	fscanf(lockPointer, "%d:%s", &pid, mode);
	fclose(lockPointer);

	if (pid != getpid()) {
		return -1;
	}

	return remove(fileLockName);
}

int waitForClearBlock(char * fileName) {
	while(isFileBlocked(fileName)) {
		char * fileLockName = getLockName(fileName);
		FILE * lockPointer = fopen(fileLockName, "r");
		int pid;
		char mode[MAX_MODE_LENGTH];

		fscanf(lockPointer, "%d:%s", &pid, mode);
		fclose(lockPointer);

		printf("Waiting. File blocked by %d, mode: %s\n", pid, mode);
		sleep(1);
	}
}

int readFile(char * fileName) {
	waitForClearBlock(fileName);
	blockFile(fileName, "read");
	
	FILE * fp;
	fp = fopen(fileName, "r");
	if (fp == NULL)
		return -1;

	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	while ((read = getline(&line, &len, fp)) != -1) {
		printf("%s", line);
		sleep(1);
	}

	fclose(fp);

	clearFileBlock(fileName);
	
	return 0;	
}

int setPassword(char * fileName, char * user, char * password) {
	waitForClearBlock(fileName);
	blockFile(fileName, "read");
	
	char users [MAX_USERS][MAX_USER_LENGTH];
	char passwords [MAX_USERS][MAX_PASSWORD_LENGTH];

	FILE * fp;
	fp = fopen(fileName, "r");

	if (fp == NULL)
		return -1;

	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	char * token;
	char * args;

	
	int i = 0;
	const char delims[3] = " \n";
	int isUserExist = 0;

	while ((read = getline(&line, &len, fp)) != -1) {
		// счиатли строку
		token = strtok(line, delims);
		strcpy(users[i], token);
		//до первого разделителя " " - user

		if (strcmp(users[i], user) == 0) {
			strcpy(passwords[i], password);
			isUserExist = 1;
		} else {
			token = strtok(NULL, delims);
			strcpy(passwords[i], token);
			//после разделителя - password
		}

		i++;

		sleep(1);

		if (i == MAX_USERS) {
			break;		
		}
	}

	fclose(fp);

	if (!isUserExist && i < MAX_USERS) {
		strcpy(users[i], user);
		strcpy(passwords[i], password);
		i++;
	}
	
	int usersLength = i;

	clearFileBlock(fileName);
	blockFile(fileName, "write");
	
	fp = fopen(fileName, "w");
	
	for (i = 0; i < usersLength; i++) {
		fprintf(fp, "%s %s\n", users[i], passwords[i]);
		sleep(1);
	}

	fclose(fp);

	clearFileBlock(fileName);
}

int main(int argc, char * argv[]) {
	if (argc == 2) {
		readFile(argv[1]);
	}
	if (argc == 4) {
		setPassword(argv[1], argv[2], argv[3]);
	}
}
