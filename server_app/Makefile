CC = gcc
CFLAGS = -Wall
LIBS = `mysql_config --cflags --libs`

all: server

server: server.o
	$(CC) $(CFLAGS) -o server server.o $(LIBS)

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

clean:
	rm -f *.o server
