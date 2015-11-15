#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#define BUFFSIZE 256
#define PACKSIZE 1040
#define ACKSIZE 16

void syserr(char *msg) { perror(msg); exit(-1); }
uint16_t CheckSum(char * packet, int psize);

int main(int argc, char *argv[])
{
  int sockfd, tempfd, portno, packetsEmpty;
  uint16_t chkSum;
  uint32_t seqNum, numPackets, expectedSeqNum;
  uint8_t ack;
  struct timeval tv;
  fd_set readfs;
  struct sockaddr_in serv_addr, clt_addr;
  socklen_t addrlen;
  char * recvIP;
  char buffer[BUFFSIZE];
  char packet[PACKSIZE];
  char ackPackage[ACKSIZE];

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
  if(sockfd < 0) syserr("Can't create socket."); 
  	printf("Creating socket...\n");

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) 
    syserr("Can't bind.");
  printf("Binding socket to port %d...\n", portno);
  
  tv.tv_sec = 1;
  tv.tv_usec = 0;
  FD_ZERO(readfs);
  FD_SET(sockfd, &readfs);
  tempfd = open(argv[2], O_CREAT | O_WRONY, 0666);
  if (tempfd < 0) syserr("Failed to open file.");
  memset(packet, 0, PACKSIZE);
  memset(ackPackage, 0, ACKSIZE);
  expectedSeqNum = 0;
  packetsEmpty = 1;

   while(1)
   {
  		select(sockfd+1, &readfds, NULL, NULL, &tv);
  		//If packet is received
  		if(FD_ISSET(sockfd, &readfds))
  		{
	  		n = recvfrom(sockfd, packet, PACSIZE, 0, 
	  		(struct sockaddr*)&clt_addr, &addrlen);
	  		if(n < 0) syserr("can't receive from sender"); 
	  
	  		seqNum = (uint32_t) packet[5] | (uint32_t) packet[4] << 8 | 
	  		   			(uint32_t) packet[3] << 16 | (uint32_t) packet[2] << 24;
	  		checksum = ChkSum(packet, PACSIZE);
	  		if(numPacketsEmpty)
	  		{
	  			numPacketsEmpty = 0;
	  			numPackets = (uint32_t) packet[10] | (uint32_t) packet[9] << 8 | 
	  								(uint32_t) packet[8] << 16 | (uint32_t) packet[7] << 24;
	  		}
	  
	  		//Check if packet is correct
	  		if(checksum == 0 && seqNum == exSeqNum)
	  		{
	  			ack = 1;
	  			//set ack
	  			ackPac[0] = ack & 255;
	  			ackPac[1] = ' ';
	  			//set seq#
	  			ackPac[2] = (seqNum >>  24) & 255;
	  			ackPac[3] = (seqNum >>  16) & 255;
			  	ackPac[4] = (seqNum >>  8) & 255;
			  	ackPac[5] = seqNum & 255;
			  	ackPac[6] = ' ';
			  	//set numPackets
			  	ackPac[7] = (numPackets >>  24) & 255;
			  	ackPac[8] = (numPackets >>  16) & 255;
			  	ackPac[9] = (numPackets >>  8) & 255;
			  	ackPac[10] = numPackets & 255;
			  	ackPac[11] = ' ';
			  	//set checksum
			  	ackPac[12] = (checksum >>  8) & 255;
			  	ackPac[13] = checksum & 255;
			  	ackPac[14] = ' ';
			  	ackPac[15] = ' ';
			  	
			  	//calculate and set checksum
			  	checksum = ChkSum(packet, PACSIZE);
			  	ackPac[12] = (checksum >>  8) & 255;
			  	ackPac[13] = checksum & 255;
	  	
			  	n = sendto(sockfd, ackPac, ACKSIZE, 0, 
			  		(struct sockaddr*)&clt_addr, addrlen);
		  		if(n < 0) syserr("can't send to receiver");
	  	
	  			exSeqNum++;
	  	
			  	//Write 1 KB packet to file
			  	if(seqNum != numPackets){
			  		n = write(tempfd, &packet[HSIZE + 1], MAXPAY);
			  		if(n < 0) syserr("can't write to file");
	  		}
	  		else	// Find where string terminates and write to file
	  		{
		  		int i;
		  		char eof;
		  		int eofLoc = MAXPAY;
		  		for(i=0; i <= MAXPAY; i++)
		  		{
		  			eof = packet[HSIZE + 1 + i];
		  			if(eof == '\0')
		  			{
		  				eofLoc  = i;
		  				break;
	  				} 
	  			}
	  			n = write(tempfd, &packet[HSIZE + 1], eofLoc);
	  			if(n < 0) syserr("Can't write at end of file");
	  		}
	  }
	  else if(exSeqNum > 0)
	  { 	
	  		//packet fault, but past first packet
	  	 	n = sendto(sockfd, ackPac, ACKSIZE, 0, 
	  		 		(struct sockaddr*)&clt_addr, addrlen);
  		 	if(n < 0) syserr("can't send to receiver");
	  }	  
	}
	else
	{
		if(seqNum != numPackets)
			printf("File not received, timeout after 60 secs.\n");
		else
			printf("File receieved, timeout after 60 secs.\n");
	}
  }
  close(sockfd);
  close(tempfd);
  return 0;
}

uint16_t CheckSum(char * packet, int psize)
{
	uint16_t checksum = 0, curr = 0, i = 0;
	while(psize > 0){
		curr = ((packet[i] << 8) + packet[i+1]) + checksum;
		checksum = curr + 0x0FFFF;
		curr = (curr >> 16); //Grab the carryout if it exists
		checksum = curr + checksum;
		psize -= 2;
		i += 2;
	}
	return ~checksum;
}
