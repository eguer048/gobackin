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
#include <arpa/inet.h>
#include <netdb.h>
#define MAXPAY 1024
#define HSIZE 15 // 1B ACK, 4B seq#, 4B numPackets, 2B checksum, 4B for spaces
#define WINSIZE 100
#define TIMEOUT 10000

void syserr(char *msg) { perror(msg); exit(-1); }
uint16_t CheckSum(char * packet, int psize);
struct Data {
   uint32_t seqNum;
   uint8_t ackNum;
   uint32_t numPackets;
   uint16_t chkSum;
   char payload[MAXPAY];
}

int main(int argc, char *argv[])
{
  int sockfd, tempfd, portno, n;
  uint32_t seqNum, numPackets, size, base;
  uint16_t checksum;
  uint8_t ack;
  struct timeval t1, t2;
  struct sockaddr_in serv_addr, clt_addr; 
  struct hostent* server; 
  struct stat filestats;
  fd_set readfds;
  socklen_t addrlen;
  char payload[MAXPAY];

  if(argc != 4) { 
    fprintf(stderr,"Usage: %s <Receiver IP> <port> <name of file>\n", argv[0]);
    return 1;
  } 
  else
  { 
	server = gethostbyname(argv[1]);
	if(!server) {
		fprintf(stderr, "ERROR: no such receiver: %s\n", argv[1]);
		return 2;
	}
  	portno = atoi(argv[2]);
  }
  
  //Socket Logic
  sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(sockfd < 0) syserr("Can't open socket."); 
  	printf("Creating socket...\n");

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr = *((struct in_addr*)server->h_addr);
  serv_addr.sin_port = htons(portno);
  addrlen = sizeof(serv_addr);
  
  //Make array of packet elements to send
  stat(argv[3], &filestats);
  size = filestats.st_size;
  numPackets = size/MAXPAY + 1;
  printf("Number of packets: %d\n", numPackets);
  Data * fileArray[numPackets];
  //char * fileArray[numPackets];
  seqNum = 0;
  checksum = 0;
  
  //Open File
  tempfd = open(argv[3], O_RDWR);
  if(tempfd < 0) syserr("Failed to open the file.");
  
  //Populate the array with packets to send
  while(1){
  	Packet * packet;
  	ack = 0;
  	packet = malloc(sizeof(struct Packet));
  	char payload[MAXPAY];
  	
  	//set ack
  	packet.ackNum = ack;
  	packet.seqNum = seqNum;
  	packet.numPackets = numPackets;
  	packet.checkSum = checksum;
  	//set seq#
  	/*packet[2] = (seqNum >>  24) & 0xFF;
  	packet[3] = (seqNum >>  16) & 0xFF;
  	packet[4] = (seqNum >>  8) & 0xFF;
  	packet[5] = seqNum & 0xFF;
  	packet[6] = ' ';
  	//set numPackets
  	packet[7] = (numPackets >>  24) & 255;
  	packet[8] = (numPackets >>  16) & 255;
  	packet[9] = (numPackets >>  8) & 255;
  	packet[10] = numPackets & 255;
  	packet[11] = ' ';
  	//set checksum
  	packet[12] = (checksum >>  8) & 255;
  	packet[13] = checksum & 255;
  	packet[14] = ' ';*/
  	
  	/*printf("The seqnum is: %d\n", seqNum);
  	uint32_t test;
  	memcpy(&test, &packet[2], sizeof(uint32_t));
  	printf("The test has: %d\n", test);
  	seqNum = (uint32_t)((uint16_t)((uint8_t) packet[5] + ((uint8_t) packet[4] << 8)) + ((uint16_t)((uint8_t) packet[3] + ((uint8_t) packet[2] << 8)) << 16));
  	printf("the seqnum bitseq is: %u, %u, %u, %u\n", (uint32_t) packet[2], (uint32_t) packet[3], (uint32_t) packet[4], (uint32_t) packet[5]);
  	printf("The rebuilt seqnum is: %u\n", seqNum);*/
  	
  	// Read payload from file, add to packet
  	int bytes_read = read(tempfd, payload, MAXPAY);
	payload[bytes_read] = '\0';
	if (bytes_read == 0) // We're done reading from the file
		break;
	if (bytes_read < 0) syserr("Error reading the file.");
	strcpy(packet.payload, payload); //Set Payload
	packet.checkSum = checksum;
	/*checksum = CheckSum(packet, HSIZE + MAXPAY + 1);
  	packet[12] = (checksum >>  8) & 255;
  	packet[13] = checksum & 255;
  	fileArray[seqNum] = packet;*/ 
  	seqNum++; 	
  }
  close(tempfd);
  
  //Set up Go-Back-N loop
  seqNum = 0;
  base = 0;
  char  * ackPac = malloc(sizeof(char)*(HSIZE + 1));
  printf("Packets ready to send!\n"); 
  
  //Send & Receive packets
  while(1)
  {
   printf("Any second now!\n");
   FD_ZERO(&readfds);
   FD_SET(sockfd, &readfds);
  	select(sockfd+1, &readfds, NULL, NULL, 0);
  	if(FD_ISSET (sockfd, &readfds)){ 	//recv acks
  	   printf("Here comes a package!");
  		n = recvfrom(sockfd, ackPac, (HSIZE + 1), 0, 
  		  (struct sockaddr*)&serv_addr, &addrlen); 
  		if(n < 0) syserr("Can't receive ack package.");
  		
  		//check if corrupt
  		checksum = CheckSum(ackPac, (HSIZE + 1));
  		if(checksum == 0){				//Not Corrupt
  			base = (uint32_t) ackPac[5] | (uint32_t) ackPac[4] << 8 | 
  			  (uint32_t) ackPac[3] << 16 | (uint32_t) ackPac[2] << 24;
  			if(base == numPackets) break; //Finished sending packets
  			base++; 					//Set base to next seqnum 
  			if (base != seqNum){
  				gettimeofday(&t1, NULL);
  			}
  		}
  		else{							//ack packet was corrupt
	  		gettimeofday(&t2, NULL);
	  		double elaps = (t2.tv_sec - t1.tv_sec) * 1000.0;
	  		elaps += (t2.tv_usec - t2.tv_usec) /1000.0;
	  		while(elaps < TIMEOUT){}	// Loop until timeout occurs
	  		if( elaps > TIMEOUT){		// clock timedout, resend packets
	  			gettimeofday(&t1, NULL);
	  			int i;
	  			for(i = base; i < seqNum; i++){
	  				n = sendto(sockfd, fileArray[i], (HSIZE + MAXPAY + 1), 0, 
	  			  		(struct sockaddr*)&serv_addr, addrlen);
	  				if(n < 0) syserr("Can't send to receiver.");
	  			}
	  		}
  		}
  	}
  	else    	//send packets
  	{								
  	   
  		if(seqNum < base + WINSIZE){	//window not maxed out
  		   
  			n = sendto(sockfd, fileArray[seqNum], (HSIZE + MAXPAY + 1), 0, 
  			  (struct sockaddr*)&serv_addr, addrlen);
  			if(n < 0) syserr("Can't send to receiver.");
  			if(n > 0)
  			{
  			   printf("Attempting to send package %d.\n", seqNum);
  			}
  			if(base == seqNum){
  				gettimeofday(&t1, NULL);
  			}
  			seqNum++;
  		}
  		gettimeofday(&t2, NULL);
  		double elaps = (t2.tv_sec - t1.tv_sec) * 1000.0;
  		elaps += (t2.tv_usec - t2.tv_usec) /1000.0;
  		printf("Attempting to send package %d.\n", seqNum);
  		if( elaps > TIMEOUT){			// clock timedout, resend packets
  			gettimeofday(&t1, NULL);
  			int i;
  			for(i = base; i < seqNum; i++){
  				n = sendto(sockfd, fileArray[i], (HSIZE + MAXPAY + 1), 0, 
  			  		(struct sockaddr*)&serv_addr, addrlen);
  				if(n < 0) syserr("Can't send to receiver.");
  			}
  		}
  	} 	
  }
  close(sockfd);
  return 0;
}

uint16_t CheckSum(Data * packet, int psize){
	uint16_t checksum = 0, curr = 0, i = 0;
	while(psize > 0){
		curr = packet.checksum;
		checksum = curr + 0x0FFFF;
		curr = (curr >> 16); //Grab the carryout if it exists
		checksum = curr + checksum;
		psize -= 2;
		i += 2;
	}
	return ~checksum;
}
