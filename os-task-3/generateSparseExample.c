#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

int main(int argc, char * argv[]) {
	int fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
	
	if (fd == -1) {
		printf("error with file");
		return -1;
	}

	write(fd, "test", 4);
	lseek(fd, 100, SEEK_CUR);
	write(fd, "test", 4);
	lseek(fd, 200, SEEK_CUR);
	write(fd, "test", 4);
	lseek(fd, 300, SEEK_CUR);
	write(fd, "test", 4);

	close(fd);

}
