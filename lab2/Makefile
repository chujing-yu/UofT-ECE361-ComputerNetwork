CC = gcc
CFLAGS = -Wall

all: deliver server

packet.o: packet.h
	$(CC) $(CFLAGS) -c packet.h -o packet.o

deliver: packet.o deliver.c
	$(CC) $(CFLAGS) packet.o deliver.c -o deliver

server: packet.o server.c
	$(CC) $(CFLAGS) packet.o server.c -o server

clean:
	rm -f packet.o deliver server
