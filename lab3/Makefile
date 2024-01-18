CC = gcc
CFLAGS = -Wall

all: deliver server

deliver: deliver.c packet.h
	$(CC) $(CFLAGS) -o deliver deliver.c

server: server.c packet.h
	$(CC) $(CFLAGS) -o server server.c

clean:
	rm -f deliver server
