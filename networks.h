//
// Written Hugh Smith, Updated: April 2020
// Use at your own risk.  Feel free to copy, just leave my name in it.
//


#ifndef __NETWORKS_H__
#define __NETWORKS_H__

#define BACKLOG 10
#define MAXBUF 1024
#define MAX_HANDLE 100
#define MAX_PACKET 1400
#define TIME_IS_NULL 1
#define TIME_IS_NOT_NULL 2
#define HEADER_LEN sizeof(struct packet_header)
#define TEST size_t size

struct packet_header{
   uint16_t length;
   uint8_t flag;
} __attribute__((packed));


// for the server side
int tcpServerSetup(int portNumber);
int tcpAccept(int server_socket, int debugFlag);

// for the client side
int tcpClientSetup(char * serverName, char * port, int debugFlag);

int selectCall(int socketNumber, int seconds, int microseconds, int timeIsNotNull);

void *smemcpy(void *start, const void *src, size_t n);
void *put_handle(void *start, uint8_t len, char *handle);


#endif
