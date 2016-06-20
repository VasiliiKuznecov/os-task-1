//fork_waitpid.c
//Запускаем несколько дочерных процессов и следим за их завершением

#include <stdio.h>
#include <sys/types.h> 
#include <sys/wait.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>

#define MAXPROC 60
#define MAX_COM_LEN 60
#define MAX_ARG_LEN 60
#define MAX_ARG_NUM 60

typedef struct process {
	char command[MAX_COM_LEN];
	char * args[MAX_ARG_NUM];
	int type;	//wait - 0, respawn - 1
	pid_t pid;
};

struct process proc_list[MAXPROC];
int proc_count = 0;
int proc_length = 0;

int getConfig() {
	FILE * fp;

	fp = fopen("launcher.conf", "r");

	if (fp == NULL)
		return -1;

	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	char * token;
	char * args;

	
	int i = 0;
	const char delims[3] = ";\n";
	const char args_delims[2] = " ";

	while ((read = getline(&line, &len, fp)) != -1) {
		// счиатли строку из конфига
		token = strtok(line, delims);
		strcpy(proc_list[i].command, token);
		//до первого разделителя ";" - название команды

		args = strtok(NULL, delims);
		//далее аргументы, их обработаем потом

		token = strtok(NULL, delims);
		//далее, тип процесса, определяем, какой и заполняем поле type
		if (strcmp(token, "wait") == 0) {
			proc_list[i].type = 0;
		} else if (strcmp(token, "respawn") == 0) {
			proc_list[i].type = 1;
		} else {
			proc_list[i].type = -1;
		}

		token = strtok(args, args_delims);
		//аргументы разобьем по пробелам

		int arg_i = 0;
		do {
			proc_list[i].args[arg_i] = (char *) malloc(MAX_ARG_LEN);
			strcpy(proc_list[i].args[arg_i], token);
			arg_i++;
		} while (token = strtok(NULL, args_delims));
		//заполнили массив аргументов

		proc_list[i].args[arg_i] = NULL;

		proc_list[i].pid = 0;

		i++;

		if (i == MAXPROC) {
			break;		
		}
	}

	fclose(fp);

	proc_length = i;
	return i;
}

void launch() {
	int i;
	for (i = 0; i < proc_length; i++) {
		int err = launchProcess(i);
	}
	printf("All processes launched\n");
}

int launchProcess(i) {
	int cpid = fork();

	switch (cpid) { 
	case -1:
		printf("Fork failed; cpid == -1\n");
		return -1;
		break;
	case 0:
		cpid = getpid();         //global PID
		printf("Process %s launched, type - %d. pid = %d\n", 
			proc_list[i].command, proc_list[i].type, cpid);
		execvp(proc_list[i].command, proc_list[i].args);
		exit(0);
	default:
		proc_list[i].pid = cpid;
		proc_count++;
		return 0;
	}
}

void watch() {
	while (proc_count) {
	    	pid_t cpid = waitpid(-1, NULL, 0);   //ждем любого завершенного потомка
		
		int i;
	    	for (i = 0; i < proc_length; i++) {
			if (proc_list[i].pid == cpid) {
				//делаем что-то по завершении дочернего процесса
			    	printf("Child number %d pid %d finished\n", i, cpid);
				proc_list[i].pid = 0;
			    	proc_count--;
				if (proc_list[i].type == 1) {
					launchProcess(i);
				}
			}
		}
	}
}

int main() {
	getConfig();

	launch();

	watch();

	return;
}
