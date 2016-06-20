#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

#define FIELD_WIDTH 20
#define FIELD_HEIGHT 20

int main(int argc, char *argv[]) {
   int sockfd, portno, n;
   struct sockaddr_in serv_addr;
   struct hostent *server;
   
   char field[FIELD_HEIGHT][FIELD_WIDTH];
   
   portno = 5001;
   
   /* Create a socket point */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }
	
   server = gethostbyname("localhost");
   
   if (server == NULL) {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
   }
   
   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   serv_addr.sin_port = htons(portno);
   
   /* Now connect to the server */
   if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR connecting");
      exit(1);
   }
      
   /* Now read server response */
   bzero(field, FIELD_HEIGHT * FIELD_WIDTH);
   n = read(sockfd, field, FIELD_HEIGHT * FIELD_WIDTH);
   
   if (n < 0) {
      perror("ERROR reading from socket");
      exit(1);
   }

   int i,j;
   for (i = 0; i < FIELD_HEIGHT; i++) {
      for (j = 0; j < FIELD_WIDTH; j++) {
         printf("%c", field[i][j] + '0');
      }
      printf("\n");
   }

   return 0;
}
