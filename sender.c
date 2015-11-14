#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netdb.h>
#define BUFFSIZE 256
#define MAXPAY 1024
#define HSIZE 15 // 1B ACK, 4B seq#, 4B numPackets, 2B checksum, 4B for spaces
#define WINSIZE 100

void syserr(char *msg) { perror(msg); exit(-1); }
uint16_t ChkSum(char * packet);

int main(int argc, char *argv[])
{
  int sockfd, newsockfd, tempfd, portno, pid, size;
  uint32_t seqNum, numPackets;
  uint16_t checksum;
  uint8_t ack;
  struct sockaddr_in serv_addr, clt_addr; 
  struct hostent* server; 
  struct stat filestats;
  socklen_t addrlen;
  char * recvIP;
  char buffer[BUFFSIZE];
  char payload[MAXPAY];

  if(argc != 4) { 
    fprintf(stderr,"Usage: %s <IP> <port> <name of file>\n", argv[0]);
    return 1;
  } 
  else
  { 
	server = gethostbyname(argv[1]);
	if(!server) {
		fprintf(stderr, "ERROR: no such host: %s\n", argv[1]);
		return 2;
	}
  	portno = atoi(argv[2]);
  }
  
  //Socket Logic
  sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(sockfd < 0) syserr("can't open socket"); 
  	printf("create socket...\n");

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr = *((struct in_addr*)server->h_addr);
  serv_addr.sin_port = htons(portno);
  
  //Make array of packet elements to send
  stat(argv[3], &filestats);
  size = filestats.st_size;
  numPackets = size/MAXPAY + 1;
  printf("num of packets: %d\n", numPackets);
  char * fileArray[numPackets];
  seqNum = 0;
  checksum = 0;
  //Open File
  tempfd = open(argv[3], O_RDWR);
  if(tempfd < 0) syserr("failed to open file");
  
  //Populate the array with packets to send
  while(1){
  	char * packet;
  	char str[13]; //CHECK THE SIZE
  	ack = 0;
  	packet = malloc(sizeof(char)*(HSIZE + MAXPAY + 1));
  	char payload[MAXPAY];
  	
  	memset(packet, 0, (HSIZE + MAXPAY));
  	//set ack
  	packet[0] = ack & 255;
  	packet[1] = ' ';
  	//set seq#
  	packet[2] = (seqNum >>  24) & 255;
  	packet[3] = (seqNum >>  16) & 255;
  	packet[4] = (seqNum >>  8) & 255;
  	packet[5] = seqNum & 255;
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
  	packet[14] = ' ';
  	
  	/*
  	int i;
  	for(i =0 ; i<16; i++){
  		printf("packet at %d is: %d\n", i, packet[i]);
  	}
  	*/
  	
  	// Read payload from file, add to packet
  	int bytes_read = read(tempfd, payload, MAXPAY);
	payload[bytes_read] = '\0';
  	//printf("bytes read are: %u. packet size is: %u. payload size: %d\n", bytes_read, sizeof(packet), sizeof(payload));
  	//printf("The string is: %s\n", packet);
	if (bytes_read == 0) // We're done reading from the file
		break;
	if (bytes_read < 0) syserr("error reading file");
	strcat(packet, payload); //Set Payload
	
	checksum = ChkSum(packet);
  	packet[12] = (checksum >>  8) & 255;
  	packet[13] = checksum & 255; 
  	seqNum++; 	
  	printf("seq num: %d", seqNum);
  }

for(;;) {
  printf("wait on port %d...\n", portno);
  addrlen = sizeof(clt_addr); 
  newsockfd = accept(sockfd, (struct sockaddr*)&clt_addr, &addrlen);
  if(newsockfd < 0) syserr("can't accept"); 
  
  pid = fork();
   if (pid < 0)
     syserr("Error on fork");
   if (pid == 0)
   {
     close(sockfd);
     ftpcomm(newsockfd, buffer);
     exit(0);
   }
   else
	 close(newsockfd);
}
  close(sockfd); 
  return 0;
}

unsigned int_to_int(unsigned k) {
    if (k == 0 || k ==1 ) return k;
    return (k % 2) + 10 * int_to_int(k / 2);
}

uint16_t ChkSum(char * packet){
	uint16_t checksum = 0, curr = 0, i = 0;
	int psize = HSIZE + MAXPAY + 1;
	//sscanf(packet, "%*s %*s %*s %u", &checksum);
	//printf("checksum is: %d. Packet Size is: %d\n", checksum, psize);
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

