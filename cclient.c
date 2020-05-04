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

#include "headers/pollLib.h"
#include "headers/safemem.h"
#include "headers/networks.h"
#include "headers/packet.h"

#define DEBUG_FLAG 1
#define PROMPT "$"

void sendToServer(int socketNum, uint8_t *buff, uint16_t sendLen);
int getFromStdin(char *sendBuf, char * prompt);
void parse_args(int argc, char * argv[]);
void init_chat(int socket, char *handle);
void chatting(int socket, char *handle);

int main(int argc, char * argv[]){
	int socketNum = 0;         //socket descriptor
	
	/* Check handle length */
	parse_args(argc, argv);

	/* set up the TCP Client socket  */
	socketNum = tcpClientSetup(argv[2], argv[3], 0);
	setupPollSet();
	init_chat(socketNum, argv[1]);
	chatting(socketNum, argv[1]);
	
	return 0;
}

/* Send flag 1 and wait for response from accept()
 * Note that eop/EOP mean end of packet
 */
void init_chat(int socket, char *handle){
	uint16_t packet_len;
	uint8_t buff[MAX_PACKET];
	int socketToProcess;
	int len;

	packet_len = build_flag1(buff, handle);
	
	addToPollSet(socket);
	sendPacket(socket, buff, packet_len);
	while(1){
		/* Wait for server to respond */
		if ((socketToProcess = pollCall(POLL_WAIT_FOREVER)) != -1){

			if(socketToProcess == socket){
				len = recvPacket(socket, CLIENT, buff);
				break;
			}
		}
	}
	if(len > 0){
		client_parse_packet(buff, socketToProcess);
		return;
	}else if(len < 0){
		fprintf(stderr, "Error with flag 1 response\n");
		exit(-1);
	}else{
		fprintf(stderr, "Server disconnected\n");
		exit(-1);
	}
}


/* Loop until client sends exit */
void chatting(int socket, char *handle){
	char input[MAX_PACKET];
	uint8_t packet[MAX_PACKET];
	int incomming_socket;
	

	addToPollSet(STDIN_FILENO);
	smemset(packet, '0', MAX_PACKET);
	while(1){

		/* Wait for server to respond */
		if ((incomming_socket = pollCall(POLL_WAIT_FOREVER)) != -1){

			/* Server socket */
			if(incomming_socket == socket){
				recvPacket(socket, CLIENT, packet);
				break;

			/* Stdin */
			}else if(incomming_socket == STDIN_FILENO){
				getFromStdin(input, PROMPT);
			}
		}
	}
}


int getFromStdin(char *sendBuf, char * prompt)
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
		if (aChar != '\n'){
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
	if (argc != 4){
		printf("usage: %s handle host-name port-number \n", argv[0]);
		exit(-1);
	}

	/* Handle */
	if(strlen(argv[1]) > MAX_HANDLE){
		printf("Handle must be <= 100 characters\n");
		exit(-1);
	}

}
