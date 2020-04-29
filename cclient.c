//
// Written Hugh Smith, Updated: April 2020
// Use at your own risk.  Feel free to copy, just leave my name in it.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "networks.h"
#include "pollLib.h"

#define DEBUG_FLAG 1

void sendToServer(int socketNum);
int getFromStdin(char * sendBuf, char * prompt);
void parse_args(int argc, char * argv[]);
void init_chat(int socket, char *handle);


int main(int argc, char * argv[])
{
	int socketNum = 0;         //socket descriptor
	
	/* Check handle length */
	parse_args(argc, argv);

	/* set up the TCP Client socket  */
	socketNum = tcpClientSetup(argv[2], argv[3], DEBUG_FLAG);
	init_chat(socketNum, argv[1]);
	sendToServer(socketNum);
	
	close(socketNum);
	
	return 0;
}

/* Send flag 1 and wait for response 
 * Note that eop/EOP mean end of packet
 */
void init_chat(int socket, char *handle){
	struct packet_header header;
	uint8_t buff[MAX_PACKET];
	uint8_t *eop = temp;
	uint8_t handle_len = strlen(handle);

	header.length = htons(HEADER_LEN 
						 		 + sizeof(handle_len) 
						 		 + handle_len);
	header.flag = 1;
	smemcpy(temp, &header, sizeof(header));
	eop = put_handle(temp + sizeof(header), handle_len, handle);
	addToPollSet(socket);
	send(socket, )
	if(pollCall(POLL_WAIT_FOREVER) < 0){
		printf("Issue with client setup\n");
		exit(-1);
	}
}

void sendToServer(int socketNum)
{
	char sendBuf[MAXBUF];   //data buffer
	int sendLen = 0;        //amount of data to send
	int sent = 0;            //actual amount of data sent/* get the data and send it   */
			
	memset(sendBuf, 0, MAXBUF);
	while (strcmp(sendBuf, "exit"))
	{

		sendLen = getFromStdin(sendBuf, "Enter data:");
		
		//printf("read: %s string len: %d (including null)\n", sendBuf, sendLen);
			
		sent =  send(socketNum, sendBuf, sendLen, 0);
		if (sent < 0)
		{
			perror("send call");
			exit(-1);
		}

		printf("Amount of data sent is: %d\n", sent);
	}
}


int getFromStdin(char * sendBuf, char * prompt)
{
	// Gets input up to MAXBUF-1 (and then appends \0)
	// Returns length of string including null
	char aChar = 0;
	int inputLen = 0;       
	
	// Important you don't input more characters than you have space 
	printf("%s ", prompt);
	while (inputLen < (MAXBUF - 1) && aChar != '\n')
	{
		aChar = getchar();
		if (aChar != '\n')
		{
			sendBuf[inputLen] = aChar;
			inputLen++;
		}
	}

	sendBuf[inputLen] = '\0';
	inputLen++;  //we are going to send the null
	
	return inputLen;
}


void parse_args(int argc, char * argv[])
{
	/* check command line arguments  */
	if (argc != 4)
	{
		printf("usage: %s handle host-name port-number \n", argv[0]);
		exit(-1);
	}

	/* Handle */
	if(strlen(argv[1]) < 0){
		printf("Handle must be <= 100 characters\n");
		exit(-1);
	}

}
