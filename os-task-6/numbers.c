#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define MAX_NUMBERS 100000

long long numbers [MAX_NUMBERS];
long long numbersLength = 0;

int byteToDigit(char byte) {
	if (byte < '0' || byte > '9') {
		return -1;
	}
	return byte - '0';
}

int getNumbers(char * fileName) {
	FILE *fp = fopen(fileName, "r");
	
	if (fp == NULL) {
		printf("Файл %s не открывается\n", fileName);
		return -1;
	}

	char byte;
	int read;
	long long currentNumber = 0;
	int isMakingNumber = 0;
	char digit;

	while ((read = fread(&byte, sizeof(char), 1, fp)) == 1) {
		digit = byteToDigit(byte);
		if (digit == -1) {
			if (isMakingNumber) {
				if (numbersLength == MAX_NUMBERS) {
					printf("Нельзя дольше записать чисел: достигнуто максимальное количесвто чисел\n");
					return 1;
				}
	
				numbers[numbersLength] = currentNumber;
				numbersLength++;

				isMakingNumber = 0;
				currentNumber = 0;
			}
		} else {
			if (isMakingNumber) {
				currentNumber = currentNumber * 10 + digit;
			} else {
				isMakingNumber = 1;
				currentNumber = digit;
			}
		}
	}

	if (isMakingNumber) {
		if (numbersLength == MAX_NUMBERS) {
			printf("Нельзя дольше записать чисел: достигнуто максимальное количесвто чисел\n");
			return 1;
		}
		
		numbers[numbersLength] = currentNumber;
		numbersLength++;
	}
}

int comp (const void * elem1, const void * elem2) 
{
    long long f = *((long long*)elem1);
    long long s = *((long long*)elem2);
    if (f > s) return  1;
    if (f < s) return -1;
    return 0;
}

int main(int argc, char * argv[]) {
	if (argc < 3) {
		printf("Недостаточно аргументов\n");
	}

	int i;

	for (i = 1; i < argc - 1; i++) {
		getNumbers(argv[i]);
	}

	qsort(numbers, numbersLength, sizeof(long long), comp);

	FILE * fp = fopen(argv[argc - 1], "w");
	
	if (fp == NULL) {
		printf("Файл %s не открывается\n", argv[argc - 1]);
		exit(1);
	}

	for (i = 0; i < numbersLength; i++) {
		int res = fprintf(fp, "%lld\n", numbers[i]);
		if (res < 0) {
			printf("Ошибка записи в файл");
			exit(1);
		}
	}
	
	return 0;
}
