
// Hugh Smith April 2017
// Network code to support TCP client/server connections
// Feel free to copy, just leave my name in it, use at your own risk.

/* Modified by Luke Matusiak */

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
#include <poll.h>

#include "headers/safemem.h"
#include "headers/gethostbyname6.h"
#include "headers/packet.h"
#include "headers/networks.h"

// This function creates the server socket.  The function 
// returns the server socket number and prints the port 
// number to the screen.

int tcpServerSetup(int portNumber)
{
	int server_socket= 0;
	struct sockaddr_in6 server;      /* socket address for local side  */
	socklen_t len= sizeof(server);  /* length of local address        */

	/* create the tcp socket  */
	server_socket = socket(AF_INET6, SOCK_STREAM, 0);
	if(server_socket < 0)
	{
		perror("socket call");
		exit(1);
	}

	// setup the information to name the socket
	server.sin6_family= AF_INET6;         		
	server.sin6_addr = in6addr_any;   //wild card machine address
	server.sin6_port= htons(portNumber);         

	// bind the name to the socket  (name the socket)
	if (bind(server_socket, (struct sockaddr *) &server, sizeof(server)) < 0)
	{
		perror("bind call");
		exit(-1);
	}
	
	//get the port number and print it out
	if (getsockname(server_socket, (struct sockaddr*)&server, &len) < 0)
	{
		perror("getsockname call");
		exit(-1);
	}

	if (listen(server_socket, BACKLOG) < 0)
	{
		perror("listen call");
		exit(-1);
	}
	
	printf("Server Port Number %d \n", ntohs(server.sin6_port));
	
	return server_socket;
}

// This function waits for a client to ask for services.  It returns
// the client socket number.   
int tcpAccept(int server_socket, int debugFlag)
{
	struct sockaddr_in6 clientInfo;   
	int clientInfoSize = sizeof(clientInfo);
	int client_socket= 0;

	if ((client_socket = accept(server_socket, (struct sockaddr*) &clientInfo, (socklen_t *) &clientInfoSize)) < 0)
	{
		perror("accept call error");
		exit(-1);
	}
	
	if (debugFlag)
	{
		printf("Client accepted.  Client IP: %s Client Port Number: %d\n",  
				getIPAddressString(clientInfo.sin6_addr.s6_addr), ntohs(clientInfo.sin6_port));
	}
	

	return(client_socket);
}

int tcpClientSetup(char * serverName, char * port, int debugFlag)
{
	// This is used by the client to connect to a server using TCP	
	int socket_num = -1;
	uint8_t * ipAddress = NULL;
	struct sockaddr_in6 server;      
	
	// create the socket
	if ((socket_num = socket(AF_INET6, SOCK_STREAM, 0)) < 0)
	{
		perror("socket call");
		exit(-1);
	}
	
	if (debugFlag)
	{
		printf("Connecting to server on port number %s\n", port);
	}
	
	// setup the server structure
	server.sin6_family = AF_INET6;
	server.sin6_port = htons(atoi(port));
	
	// get the IP address of the server (DNS lockup)
	if ((ipAddress = getIPAddress6(serverName, &server)) == NULL)
	{
		exit(-1);
	}

	//printf("server ip address: %s\n", getIPAddressString(ipAddress));
	if(connect(socket_num, (struct sockaddr*)&server, sizeof(server)) < 0)
	{
		perror("connect call");
		exit(-1);
	}

	if (debugFlag)
	{
		printf("Connected to %s IP: %s Port Number: %d\n", serverName, getIPAddressString(ipAddress), atoi(port));
	}
	
	return socket_num;
}


void sendPacket(int socketNum, uint8_t *sendBuf, uint16_t sendLen)
{
	uint16_t sent = 0;           //actual amount of data sent */

	//sendLen = getFromStdin(sendBuf, "Enter data:");
	//printf("read: %s string len: %d (including null)\n", sendBuf, sendLen);	
	sent =  (uint16_t)send(socketNum, sendBuf, sendLen, MSG_WAITALL);
	if (sent != sendLen)
	{
		perror("send call");
		exit(-1);
	}
}


int recvPacket(int clientSocket, int process_type, uint8_t *return_buff)
{
	uint8_t buf[MAX_PACKET];
	int messageLen = 0;
	
	/* 0 out buffers */
	smemset(buf, '0', MAX_PACKET);
	smemset(return_buff, '0', MAX_PACKET);
	
	/* Get data */
	if ((messageLen = recv(clientSocket, buf, MAX_PACKET, 0)) < 0)
	{
		perror("recv call");
		exit(-1);
	}
	
	if (messageLen == 0)
	{
		/* recv() 0 bytes so client is gone */
		if(process_type == SERVER){
			return(0);

		/* Server is gone */
		}else{
			printf("Server sent 0 bytes, exiting\n");
			exit(-1);
		}

	}
	/* Copy data */
	smemcpy(return_buff, buf, messageLen);
	return(messageLen);
}