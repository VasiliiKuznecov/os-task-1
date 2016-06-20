#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h> 

#define MAX_WORDS 100
#define MAX_WORD_LENGTH 100

int pipefdHandlersOutput [MAX_WORDS];

int isPalindrome(char * word) {
	int length = strlen(word);
	int halfLength = length / 2;
	int i;
	
	for (i = 0; i < halfLength; i++) {
		if (word[i] != word[length - 1 - i]) {
			return 0;
		}
	}

	return 1;
}

int readWords(char * fileName, char words [MAX_WORDS][MAX_WORD_LENGTH]) {
	FILE * fp = fopen(fileName, "r");

	if (fp == NULL)
		return -1;

	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	char * token;
	char * args;

	
	int i = 0;
	const char delims[3] = " \n";

	while ((read = getline(&line, &len, fp)) != -1) {
		// счиатли строку
		token = strtok(line, delims);
		strcpy(words[i], token);
		//до первого разделителя слово

		i++;

		if (i == MAX_WORDS) {
			break;		
		}
	}

	fclose(fp);

	return i;
}

int handleWord(int inputfd, int outputfd) {
	fd_set readfds;

	FD_ZERO(&readfds);
	FD_SET(inputfd, &readfds);

	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;

	int selectResult = select(inputfd + 1, &readfds, NULL, NULL, &timeout);
	
	if (selectResult == -1) {
		printf("Select failed\n");

		exit(1);
	}

	FILE * inputfp = fdopen(inputfd, "r");
	char word [MAX_WORD_LENGTH];
	
	fscanf(inputfp, "%s", word);
	fclose(inputfp);

	FILE * outputfp = fdopen(outputfd, "w");
	fprintf(outputfp, "%d\n", isPalindrome(word));
	fclose(outputfp);	

	return 0;
}

int launchWordHandler(int index, char * word) {
	int pipefdHandlerInput [2];
	pipe(pipefdHandlerInput);

	int pipefdHandlerOutput [2];
	pipe(pipefdHandlerOutput);

	FILE * fp;
	
	int cpid = fork();

	switch (cpid) { 
	case -1:
		printf("Fork failed; cpid == -1\n");

		exit(1);
	case 0:
		handleWord(pipefdHandlerInput[0], pipefdHandlerOutput[1]);

		exit(0);
	default:
		fp = fdopen(pipefdHandlerInput[1], "w");
		fprintf(fp, "%s\n", word);
		fclose(fp);

		pipefdHandlersOutput[index] = pipefdHandlerOutput[0];

		return 0;
	}


}

int parallelPalindrome(char * fileName) {
	char words [MAX_WORDS][MAX_WORD_LENGTH];
	int wordsNumber = readWords(fileName, words);
	int i;

	for(i = 0; i < wordsNumber; i++) {
		launchWordHandler(i, words[i]);
	}
	
	for(i = 0; i < wordsNumber; i++) {
		fd_set readfds;

		FD_ZERO(&readfds);
		FD_SET(pipefdHandlersOutput[i], &readfds);

		struct timeval timeout;
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;

		int selectResult = select(pipefdHandlersOutput[i] + 1, &readfds, NULL, NULL, &timeout);
	
		if (selectResult == -1) {
			printf("Select failed\n");

			exit(1);
		}
	
		FILE * fp = fdopen(pipefdHandlersOutput[i], "r");
		int answer;
	
		fscanf(fp, "%d", &answer);
	
		if (answer == 1) {
			printf("%s is palindrome\n", words[i]);
		} else {
			printf("%s is not palindrome\n", words[i]);
		}
	}
}

int main(int argc, char * argv[]) {
	if (argc == 2) {
		parallelPalindrome(argv[1]);
	}
}
