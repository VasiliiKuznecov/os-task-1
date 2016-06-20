#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>

#include <netdb.h>
#include <netinet/in.h>

#define FIELD_WIDTH 20
#define FIELD_HEIGHT 20

char* shared_memory;
int segment_id;
struct shmid_ds shmbuffer;
int segment_size;
const int shared_segment_size = FIELD_WIDTH * FIELD_HEIGHT + 2;

char field[FIELD_HEIGHT][FIELD_WIDTH] = {
	0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

};

int waitForSharedMemoryRead() {
	while(shared_memory[0] == 1) {
		;
	}
}

int waitForSharedMemoryWrite() {
	while(shared_memory[0] == 2) {
		;
	}
}

int fillSharedMemory() {
	waitForSharedMemoryRead();
	shared_memory[0] = 2;

	int i,j;

	for (i = 0; i < FIELD_HEIGHT; i++) {
		for (j = 0; j < FIELD_WIDTH; j++) {
			shared_memory[i * FIELD_WIDTH + j + 1] = field[i][j];
		}
	}
	
	shared_memory[0] = 0;
}

int fillField() {
	waitForSharedMemoryWrite();
	shared_memory[0] = 1;

	int i,j;

	for (i = 0; i < FIELD_HEIGHT; i++) {
		for (j = 0; j < FIELD_WIDTH; j++) {
			field[i][j] = shared_memory[i * FIELD_WIDTH + j + 1];
		}
	}	

	if (shared_memory[0] == 1) {
		shared_memory[0] = 0;
	}
}

int getNeighboursNumber(int i, int j) {
	int shiftI, shiftJ;
	int neighboursNumber = 0;
	int neighbourI, neighbourJ;

	for (shiftI = -1; shiftI <= 1; shiftI++) {
		for (shiftJ = -1; shiftJ <= 1; shiftJ++) {
			if (shiftI == 0 && shiftJ == 0) {
				continue;
			}

			neighbourI = (i + shiftI + FIELD_HEIGHT) % FIELD_HEIGHT;

			neighbourJ = (j + shiftJ + FIELD_WIDTH) % FIELD_WIDTH;

			neighboursNumber += field[neighbourI][neighbourJ];
		}
	}
	
	return neighboursNumber;
}

int makeStep() {
	fillField();
	
	int i,j;

	waitForSharedMemoryRead();

	shared_memory[0] = 2;

	for (i = 0; i < FIELD_HEIGHT; i++) {
		for (j = 0; j < FIELD_WIDTH; j++) {
			int neighboursNumber = getNeighboursNumber(i, j);
			if (field[i][j] == 1) {
				if (neighboursNumber == 2 || neighboursNumber == 3) {
					shared_memory[i * FIELD_WIDTH + j + 1] = 1;
				} else {
					shared_memory[i * FIELD_WIDTH + j + 1] = 0;
				}
			} else {
				if (neighboursNumber == 3) {
					shared_memory[i * FIELD_WIDTH + j + 1] = 1;
				} else {
					shared_memory[i * FIELD_WIDTH + j + 1] = 0;
				}
			}
		}

	}

	shared_memory[0] = 0;

}

int runMakeStep() {
	int cpid = fork();

	switch (cpid) { 
	case -1:
		printf("Fork failed; cpid == -1\n");
		exit(1);
		break;
	case 0:
		shared_memory[FIELD_HEIGHT * FIELD_WIDTH + 1] = 0;
		makeStep();
		shared_memory[FIELD_HEIGHT * FIELD_WIDTH + 1] = 1;
		exit(0);
	default:
		return cpid;
	}
	
}

int runLife() {
	fillSharedMemory();

	int cpid;

	while (1) {
		cpid = runMakeStep();
		sleep(1);

		if (shared_memory[FIELD_HEIGHT * FIELD_WIDTH + 1] == 0) {
			printf("Не успели посчитать поле за секунду\n");
		}

		wait();

		if (kill(getppid, 0) == 0) {
			exit(0);
		}

	}
}

int runServer() {
	int sockfd, newsockfd, portno, clilen;
	struct sockaddr_in serv_addr, cli_addr;
	int  n;
   
	/* First call to socket() function */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
	if (sockfd < 0) {
		perror("ERROR opening socket");
		exit(1);
	}
   
	/* Initialize socket structure */
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = 5001;
   
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
   
	/* Now bind the host address using bind() call.*/
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("ERROR on binding");
		exit(1);
	}
      
	/* Now start listening for the clients, here process will
	* go in sleep mode and will wait for the incoming connection
	*/
   
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
   	
	while(1) {
		/* Accept actual connection from the client */
		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

		printf("accepted connection\n");
	
		if (newsockfd < 0) {
			perror("ERROR on accept");
			continue;
		}

		int cpid = fork();

		switch (cpid) { 
		case -1:
			printf("Fork failed; cpid == -1\n");
			exit(1);
			break;
		case 0:
			close(sockfd);
			/* Write a response to the client */
			shared_memory[0] = 1;

			n = write(newsockfd, shared_memory + 1, FIELD_HEIGHT * FIELD_WIDTH);

			if (shared_memory[0] != 2) {
				shared_memory[0] = 0;
			}
		   
			if (n < 0) {
				perror("ERROR writing to socket");
				exit(1);
			}

			close(newsockfd);
			exit(0);

		default:
			close(newsockfd);
		}

	   
	}
}

int runProgram() {
	int cpid = fork();

	switch (cpid) { 
	case -1:
		printf("Fork failed; cpid == -1\n");
		exit(1);
		break;
	case 0:

		runLife();
	default:
		runServer();
	}


}
 
int main () {
	/*Выделение общей памяти, в 1ом байте: 
		если 0 - общая память в данный момент не используется,
		1 - читается,
		2 - пишется,
	следующие 400 байт - таблица игры
	последний байт - флаг, посчитался ли следующий шаг*/

	/* Allocate a shared memory segment.  */
	segment_id = shmget (IPC_PRIVATE, shared_segment_size, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

	/* Attach the shared memory segment.  */
	shared_memory = (char*) shmat (segment_id, 0, 0);
	/*shared memory attached at address shared_memory*/

	/* Determine the segment's size. */
	shmctl (segment_id, IPC_STAT, &shmbuffer);
	segment_size  = shmbuffer.shm_segsz;

	runProgram();

	/* Detach the shared memory segment.  */
	shmdt (shared_memory);

	/* Deallocate the shared memory segment.  */
	shmctl (segment_id, IPC_RMID, 0);

	return 0;
}
