CC= gcc
CFLAGS= -g -Wall

all: cclient server

cclient: safemem networks pollLib gethost table
	$(CC) $(CFLAGS) -o cclient cclient.c safemem.o networks.o pollLib.o gethostbyname6.o

server: safemem networks pollLib gethost table
	$(CC) $(CFLAGS) -o server server.c safemem.o networks.o pollLib.o gethostbyname6.o table.o

safemem:
	$(CC) $(CFLAGS) -c safemem.c

networks: safemem
	$(CC) $(CFLAGS) -c networks.c

pollLib:
	$(CC) $(CFLAGS) -c pollLib.c

gethost:
	$(CC) $(CFLAGS) -c gethostbyname6.c

table: safemem
	$(CC) $(CFLAGS) -c table.c

table_test: table 
	$(CC) $(CFLAGS) -o test test_table.c table.o safemem.o


clean:
	rm -f server cclient *.o *.gch test
