#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#define PACSIZE 1040
#define MAXPAY 1024
#define ACKSIZE 16
#define HSIZE 15

struct Data {
   uint32_t seqNum;
   uint8_t ackNum;
   uint32_t numPackets;
   uint16_t checkSum;
   char payload[MAXPAY];
};
void syserr(char *msg) { perror(msg); exit(-1); }
uint16_t CheckSum(struct Data * packet);

int main(int argc, char *argv[])
{
  int sockfd, tempfd, portno, n;
  uint32_t seqNum, expectedSeqNum, numPackets;
  uint16_t checksum;
  uint8_t ack;
  struct sockaddr_in serv_addr, clt_addr;
  struct timeval tv;
  fd_set readfds;
  socklen_t addrlen;
  struct Data * packet; 
  struct Data * ackPac;

  if(argc != 3) { 
    fprintf(stderr,"Usage: %s <port> <name of file>\n", argv[0]);
    return 1;
  } 
  else
  { 
  	portno = atoi(argv[1]);
  }

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sockfd < 0) syserr("Can't open the socket."); 
  	printf("Creating socket...\n");

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  addrlen = sizeof(clt_addr); 

  if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) 
    syserr("Can't bind.");
  printf("Binding socket to port %d...\n", portno);
  
  //Set up to receive
  tv.tv_sec = 60;
  tv.tv_usec = 0;
  tempfd = open(argv[2], O_CREAT | O_WRONLY, 0666);
  if(tempfd < 0) syserr("Failed to open the file.");
  expectedSeqNum = 0;
  
  //Receive packet
  while(1){
   printf("Waiting on port %d...\n", portno);
   FD_ZERO(&readfds);
   FD_SET(sockfd, &readfds);
  	select(sockfd+1, &readfds, NULL, NULL, &tv);
  	
  	//If packet is received
  	if(FD_ISSET(sockfd, &readfds)){
	  n = recvfrom(sockfd, packet, sizeof(struct Data), 0, 
	  	(struct sockaddr*)&clt_addr, &addrlen); 
	  if(n < 0) syserr("Can't receive from sender.");

     seqNum = packet->seqNum;
	  checksum = CheckSum(packet);
	  ack = packet->ackNum;
	  numPackets = packet->numPackets;
	  
	  //Check if packet is correct
	  if(checksum == 0 && seqNum == expectedSeqNum)
	  {
	  	
	  	ackPac->ackNum = ack;
	  	//set seq#
	  	ackPac->seqNum = seqNum;
	  	//set numPackets
	  	ackPac->numPackets = numPackets;
	  	//calculate and set checksum
	  	ackPac->checkSum = checksum;
	  	strcpy(ackPac->payload, packet->payload);
	  	
	  	n = sendto(sockfd, ackPac, sizeof(struct Data), 0, 
	  		(struct sockaddr*)&clt_addr, addrlen);
  		if(n < 0) syserr("Can't send to receiver.");
	  	
	  	expectedSeqNum++;
	  	
	  	//Write 1 KB packet to file
	  	if(seqNum != numPackets)
	  	{
	  		n = write(tempfd, &packet->payload, MAXPAY);
	  		if(n < 0) syserr("Can't write to file.");
	  	}
	  	else
	  	{
	  		// Find where string terminates and write to file
	  		int i;
	  		char eof;
	  		int eofLoc = MAXPAY;
	  		for(i=0; i <= MAXPAY; i++)
	  		{
	  			eof = packet->payload[i];
	  			if(eof == '\0')
	  			{
	  				eofLoc  = i;
	  				break;
	  			} 
	  		}
	  		n = write(tempfd, &packet->payload, eofLoc);
	  		if(n < 0) syserr("Can't write at the end of file.");
	  	}
	  	
	  }
	  else if(expectedSeqNum > 0)	//packet fault, but past first packet
	  { 	
	  	 n = sendto(sockfd, ackPac, sizeof(struct Data), 0, 
	  		 (struct sockaddr*)&clt_addr, addrlen);
  		 if(n < 0) syserr("Can't send to receiver.");
	  }
	  
	}
	else
	{
		if(seqNum != numPackets)
			printf("File not received, timeout after 60 secs.\n");
		else
			printf("File receieved, timeout after 60 secs.\n");
		break;
	}
  }
  
  close(sockfd); 
  close(tempfd);
  return 0;
}

uint16_t CheckSum(struct Data * packet){
	uint16_t checksum = 0;
	if (packet->checkSum == 0)
		checksum = 255;
	return ~checksum;
}
