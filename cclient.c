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

#include "pollLib.h"
#include "safemem.h"
#include "networks.h"
#include "packet.h"

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
void parse_B(char *input);
int break_msg(char *msg, char messages[MAX_PACKET/MAX_MSG][MAX_MSG]);

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
	
	if(input[0] != '%'){ 
		printf("\nInvalid Command\n$ "); 
		fflush(stdout);
		return; 
	}

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
			break;
		case 'B':
		case 'b':
			parse_B(input);
			break;
		default:
			printf("\nInvalid Command\n$ ");
			break;
	}

	fflush(stdout);
}


void parse_B(char *input){
	char messages[NUM_MSGS][MAX_MSG];
	uint8_t buff[MAX_PACKET] = "";
	uint16_t len;
	char *ptr;
	int num_msgs;
	int i;

	/* Clear %b */
	ptr = strtok(input, " ");

	/* Check for empty message */
	if((ptr=strtok(NULL, "\0")) == NULL){
		len = build_flag4(buff, "", my_handle);
		sendPacket(my_socket, buff, len);
		return;
	}

	num_msgs = break_msg(ptr, messages);

	for(i = 0; i < num_msgs; i++){
		len = build_flag4(buff, messages[i], my_handle);
		sendPacket(my_socket, buff, len);
		memset(buff, '0', MAX_PACKET);
	}

}

/* Request list of handles by sending flag 10 */
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
	char msg[MAX_PACKET] = "";
	char messages[NUM_MSGS][MAX_MSG];
	int num_msgs;
	uint8_t buff[MAX_PACKET] = "";
	uint8_t num_handles;
	char delim[] = " ";
	char *ptr;
	uint16_t packet_length;

	int i;

	/* take %m token */
	ptr = strtok(input, delim);

	/* Check for length token */
	if((ptr = strtok(NULL, delim)) == NULL){ printf("Invalid format\n"); return; }
	if((num_handles = atoi(ptr)) <= 0){ printf("Invalid format\n"); return; }
	if(num_handles > MAX_NUM_HANDLES){ printf("Invalid format\n"); return; }
	
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

	/*Empty Message */
	if((ptr = strtok(NULL, delim)) == NULL){
		packet_length = build_flag5(buff, my_handle, handles, num_handles, "");
		sendPacket(my_socket, buff, packet_length);
		return;
	}

	sstrcpy(msg, ptr);
	num_msgs = break_msg(msg, messages);
	for(i = 0; i < num_msgs; i++){
		packet_length = build_flag5(buff, my_handle, handles, num_handles, messages[i]);
		sendPacket(my_socket, buff, packet_length);
		memset(buff, '0', sizeof(buff));
	}
}


int break_msg(char *msg, char messages[NUM_MSGS][MAX_MSG]){
	int msg_len;
	int i;

	/* Clear out arrays, guarenteed to have a null byte for string */
	for(i = 0; i < NUM_MSGS; i++){
		smemset(messages[i], '\0', sizeof(messages[i]));
	}

	/* Message will fit into one packet */
	if((msg_len = sstrlen(msg)) < MAX_MSG){
		sstrcpy(messages[0], msg);
		return 1;
	}

	i = 0;
	while(msg_len > 0){
		if(msg_len < MAX_MSG){
			smemcpy(messages[i], msg, msg_len);
			break;
		}

		smemcpy(messages[i], msg, MAX_MSG - 1);
		msg += (MAX_MSG - 1);
		msg_len -= (MAX_MSG - 1);
		i++;
	}
	return i + 1;
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
