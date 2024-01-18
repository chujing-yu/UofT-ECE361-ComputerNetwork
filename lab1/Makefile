CC = gcc
CFLAGS = -Wall -g
DELIVER_TARGET = deliver_program
SERVER_TARGET = server_program

all: $(DELIVER_TARGET) $(SERVER_TARGET)

$(DELIVER_TARGET): deliver.c
	$(CC) $(CFLAGS) $^ -o $(DELIVER_TARGET)

$(SERVER_TARGET): server.c
	$(CC) $(CFLAGS) $^ -o $(SERVER_TARGET)

clean:
	rm -f $(DELIVER_TARGET) $(SERVER_TARGET)

