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
void chatting();
void parse_input(int len, char *input);
void parse_M(int len, char *input);
void parse_E();
void parse_L();

char my_handle[MAX_HANDLE];
int my_socket;

int main(int argc, char * argv[]){
	
	parse_args(argc, argv);

	my_socket = tcpClientSetup(argv[2], argv[3], 0);
	setupPollSet();
	init_chat(my_socket, argv[1]);
	printf(PROMPT);
	fflush(stdout);
	chatting();
	
	return 0;
}

/* Send flag 1 and wait for response from server */
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
		if ((socketToProcess = pollCall(0)) != -1){

			if(socketToProcess == socket){
				len = recvPacket(socket, CLIENT, buff);
				break;
			}
		}
	}
	if(len > 0){
		client_parse_packet(buff, socketToProcess);
		strcpy(my_handle, handle);
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
void chatting(){
	char input[MAX_PACKET];
	uint8_t packet[MAX_PACKET];
	int incomming_socket;
	int input_len;

	addToPollSet(STDIN_FILENO);
	smemset(packet, '\0', MAX_PACKET);
	smemset(input, '\0', MAX_PACKET);
	
	while(1){

		/* Wait for server to respond */
		if ((incomming_socket = pollCall(0)) != -1){

			/* Server socket */
			if(incomming_socket == my_socket){
				recvPacket(my_socket, CLIENT, packet);
				client_parse_packet(packet, my_socket);	/* located in packet.c */
				smemset(packet, '\0', MAX_PACKET);

			/* Stdin */
			}else if(incomming_socket == STDIN_FILENO){
				if((input_len = getFromStdin(input, PROMPT)) == -1){
					continue;
				}
				parse_input(input_len, input);
				smemset(packet, '\0', MAX_PACKET);
				smemset(input, '\0', MAX_PACKET);
			}
		}

	}
}


int getFromStdin(char *sendBuf, char * prompt){
	char aChar = 0;
	int inputLen = 0;       
	
	// Important you don't input more characters than you have space 
	printf("%s ", prompt);
	while (inputLen <= (MAX_PACKET + 1) && aChar != '\n')
	{
		if(inputLen == MAX_PACKET + 1){
			fprintf(stderr, "Input too long\n");
			return(-1);
		}
		aChar = getchar();
		if (aChar != '\n'){
			sendBuf[inputLen] = aChar;
			inputLen++;
		}
	}
	fflush(stdout);
	return inputLen;
}


void parse_input(int len, char *input){
	
	if(input[0] != '%'){ fflush(stdout); return; }

	switch(input[1]){
		case 'M':
		case 'm':
			parse_M(len, input);
			break;
		case 'E':
		case 'e':
			parse_E();
			break;
		case 'L':
		case 'l':
			parse_L();
		default:
			break;
	}

	fflush(stdout);
}


/* Request list of handles */
void parse_L(){
	int hrequest = 10;
	struct packet_header header;

	header = build_header(hrequest);
	sendPacket(my_socket, (uint8_t *)&header, ntohs(header.length));
}


/* Send exit flag */
void parse_E(){
	struct packet_header header;
	int exit_flag = 8;

	header = build_header(exit_flag);
	sendPacket(my_socket, (uint8_t *)&header, ntohs(header.length));
}


/* Assumes proper formatting. Will fail otherwise */
void parse_M(int len, char *input){
	char handles[MAX_NUM_HANDLES][MAX_HANDLE + 1];
	char msg[MAX_MSG] = "";
	//int msg_len;
	uint8_t buff[MAX_PACKET] = "";
	uint8_t num_handles;
	char delim[] = " ";
	char *ptr;
	uint16_t packet_length;
	//int breakc;
	//int leftover;
	int i;

	/* take %m token */
	ptr = strtok(input, delim);

	/* Check for length token */
	if((ptr = strtok(NULL, delim)) == NULL){ return; }
	if((num_handles = atoi(ptr)) <= 0){ return; }
	if(num_handles > MAX_NUM_HANDLES){ return; }
	
	/* Get handles */
	for(i = 0; i < num_handles; i++){
		smemset(handles[i], '\0', sizeof(handles[i]));
		if((ptr = strtok(NULL, delim)) == NULL){ return; }
		if(strlen(ptr) > MAX_HANDLE){
			printf("\nHandle %s is too long\n$ ", ptr);
			fflush(stdout);
			return;
		}
		strcpy(handles[i], ptr);
	}

	/* Get message */
	delim[0] = '\0';
	if((ptr = strtok(NULL, delim)) == NULL){ return; }
	/*if((msg_len = strlen(ptr)) >= MAX_MSG){
		breakc = msg_len / MAX_MSG;
		leftover = msg_len % MAX_MSG;

		for(i = 0; i < breakc; i++){
			smemcpy(msg, ptr, MAX_MSG - 1);   minus 1 for null 
			packet_length = build_flag5

		}
	}
	*/
	strcpy(msg, ptr);
	packet_length = build_flag5(buff, my_handle, handles, num_handles, msg);
	sendPacket(my_socket, buff, packet_length);
	return;
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
