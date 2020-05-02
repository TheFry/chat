CC= gcc
CFLAGS= -g -Wall

all: cclient server

cclient: safemem.o networks.o pollLib.o gethostbyname6.o table.o
	$(CC) $(CFLAGS) -o cclient cclient.c safemem.o networks.o pollLib.o gethostbyname6.o

server: safemem.o networks.o pollLib.o gethostbyname6.o table.o
	$(CC) $(CFLAGS) -o server server.c safemem.o networks.o pollLib.o gethostbyname6.o table.o

safemem.o:
	$(CC) $(CFLAGS) -c safemem.c -o safemem.o

networks.o: safemem.o
	$(CC) $(CFLAGS) -c networks.c -o networks.o

pollLib.o:
	$(CC) $(CFLAGS) -c pollLib.c

gethostbyname6.o:
	$(CC) $(CFLAGS) -c gethostbyname6.c

table.o:
	$(CC) $(CFLAGS) -c table.c

test_table: table.o safemem.o
	$(CC) $(CFLAGS) -o test_table test_table.c table.o safemem.o


clean:
	rm -f server cclient test_table *.o *.gch
