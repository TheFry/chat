CC= gcc
CFLAGS= -g -Wall -pedantic

all: cclient server

cclient: safemem.o networks.o pollLib.o gethostbyname6.o table.o packet.o cclient.o
	$(CC) $(CFLAGS) -o cclient cclient.o safemem.o networks.o pollLib.o gethostbyname6.o table.o packet.o

server: safemem.o networks.o pollLib.o gethostbyname6.o table.o packet.o server.o
	$(CC) $(CFLAGS) -o server server.o safemem.o networks.o pollLib.o gethostbyname6.o table.o packet.o

safemem.o:
	$(CC) $(CFLAGS) -c safemem.c -o safemem.o

networks.o: safemem.o
	$(CC) $(CFLAGS) -c networks.c -o networks.o

pollLib.o:
	$(CC) $(CFLAGS) -c pollLib.c -o pollLib.o

gethostbyname6.o:
	$(CC) $(CFLAGS) -c gethostbyname6.c -o gethostbyname6.o

table.o: networks.o
	$(CC) $(CFLAGS) -c table.c -o table.o

packet.o: 
	$(CC) $(CFLAGS) -c packet.c -o packet.o

test_table: table.o safemem.o
	$(CC) $(CFLAGS) -o test/test_table test/test_table.c table.o safemem.o

test_client: cclient safemem.o
	$(CC) $(CFLAGS) -o test/test_client test/multi_client.c safemem.o

clean:
	rm -f server cclient test/test_table test/test_client

clean-hard: clean
	rm -f *.o *.gch
	rm -f vgcore*
