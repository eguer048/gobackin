#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#define BUFFSIZE 256

void syserr(char *msg) { perror(msg); exit(-1); }

int main(int argc, char *argv[])
{
  int sockfd, newsockfd, portno, pid;
  struct sockaddr_in serv_addr, clt_addr;
  socklen_t addrlen;
  char * recvIP;
  char buffer[256];

  if(argc != 3) { 
    fprintf(stderr,"Usage: %s <port> <name of file>\n", argv[0]);
    return 1;
  } 
  else
  { 
	server = gethostbyname(argv[1]);
	if(!server) 
	{
		fprintf(stderr, "ERROR: no such host: %s\n", argv[1]);
		return 2;
	}
  	portno = atoi(argv[1]);
  }

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sockfd < 0) syserr("can't open socket"); 
  	printf("create socket...\n");

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) 
    syserr("can't bind");
  printf("bind socket to port %d...\n", portno);

for(;;) 
{
  
}
