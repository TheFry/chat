CC= gcc
CFLAGS= -g -Wall

all: cclient

cclient: safemem networks pollLib gethost
	$(CC) $(CFLAGS) -o cclient cclient.c safemem.o networks.o pollLib.o gethostbyname6.o

safemem:
	$(CC) $(CFLAGS) -c safemem.c

networks:
	$(CC) $(CFLAGS) -c networks.c

pollLib:
	$(CC) $(CFLAGS) -c pollLib.c

gethost:
	$(CC) $(CFLAGS) -c gethostbyname6.c


clean:
	rm -f server cclient *.o *.gch
