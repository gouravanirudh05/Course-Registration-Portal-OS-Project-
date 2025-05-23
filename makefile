# Compiler and flags
CC = gcc
# Targets
all: server client

server: server.c server_utils.c server_utils.h
	$(CC) server.c server_utils.c -o server -lpthread

client: client.c
	$(CC) client.c -o client

run-server: server
	./server

run-client: client
	./client

clean:
	rm -f server client
