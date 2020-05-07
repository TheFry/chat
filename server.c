//
// Written Hugh Smith, Updated: April 2020
// Use at your own risk.  Feel free to copy, just leave my name in it.
// Modified by Luke Matusiak


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

#include "headers/safemem.h"
#include "headers/networks.h"
#include "headers/pollLib.h"
#include "headers/table.h"
#include "headers/packet.h"


#define DEBUG_FLAG 1

void processSockets(int mainServerSocket);
void recvFromClient(int clientSocket);
void addNewClient(int mainServerSocket);
void removeClient(int clientSocket);
int checkArgs(int argc, char *argv[]);


int main(int argc, char *argv[]){
	int mainServerSocket = 0;   //socket descriptor for the server socket
	int portNumber = 0;
	
	setupPollSet();
	init_table();
	portNumber = checkArgs(argc, argv);
	
	//create the server socket
	mainServerSocket = tcpServerSetup(portNumber);

	// Main control process (clients and accept())
	processSockets(mainServerSocket);
	
	// close the socket - never gets here but nice thought
	close(mainServerSocket);
	
	return 0;
}


void processSockets(int mainServerSocket){
	int socketToProcess;
	int len;
	uint8_t buff[MAX_PACKET];
	addToPollSet(mainServerSocket);

	smemset(buff, '0', sizeof(buff));
	socketToProcess = -1;
	/* Run until user inputs ^C */	
	while (1){

		if ((socketToProcess = pollCall(0)) != -1){
			
			/* New client */
			if (socketToProcess == mainServerSocket){
				addNewClient(mainServerSocket);
				socketToProcess = -1;
				continue;
			}
			
			/* Get client data */
			len = recvPacket(socketToProcess, SERVER, buff);

			/* Close connection if 0 */
			if(!len){
				removeClient(socketToProcess);
				socketToProcess = -1;
				continue;
			}
			
			/* Parse the packet */
			server_parse_packet(buff, socketToProcess);	
			memset(buff, '0', sizeof(buff));
			socketToProcess = -1;

		}else{
			// Just printing here to let me know what is going on
			/*printf("Poll timed out waiting for client to send data\n");*/
		}
		
	}
}


void addNewClient(int mainServerSocket){
	int newClientSocket = tcpAccept(mainServerSocket, DEBUG_FLAG);
	addToPollSet(newClientSocket);
}


/* Remove client from handle table and poll set */
void removeClient(int clientSocket){
	printf("Client %d: Terminted\n", clientSocket);

	if(remove_entry(clientSocket)){
		fprintf(stderr, "Socket %d was not in table\n", clientSocket);
	}

	removeFromPollSet(clientSocket);
	close(clientSocket);
}


int checkArgs(int argc, char *argv[]){
	// Checks args and returns port number
	int portNumber = 0;

	if (argc > 2){
		fprintf(stderr, "Usage %s [optional port number]\n", argv[0]);
		exit(-1);
	}
	
	if (argc == 2){
		portNumber = atoi(argv[1]);
	}
	
	return portNumber;
}

