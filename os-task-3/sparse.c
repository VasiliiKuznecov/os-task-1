#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define BUFFER_LENGTH 100


int makeSparse(int fd) {
	char buffer[BUFFER_LENGTH];
	char writeBuffer[BUFFER_LENGTH];
	int bytesRead = 0;
	int writeBufferLength = 0;
	int zeroesLength = 0;
	long fileSize = 0;

	while(bytesRead = read(0, buffer, BUFFER_LENGTH)) {
		int i = 0;

		fileSize += bytesRead;

		while (i < bytesRead) {
			writeBufferLength = 0;		
			while(i < bytesRead && buffer[i] != 0) {
				writeBuffer[writeBufferLength] = buffer[i];
				writeBufferLength++;
				i++;
			}
			if (writeBufferLength > 0) {
				write(fd, writeBuffer, writeBufferLength);
			}
	
			zeroesLength = 0;
			while(i < bytesRead && buffer[i] == 0) {
				zeroesLength++;
				i++;
			}
			if (zeroesLength > 0) {
				lseek(fd, zeroesLength, SEEK_CUR);
			}
		}

	}
	
	ftruncate(fd, fileSize);

	return 0;
}

int main(int argc, char * argv[]) {
	int fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
	
	if (fd == -1) {
		printf("error with file");
		return -1;
	}

	return makeSparse(fd);
}
