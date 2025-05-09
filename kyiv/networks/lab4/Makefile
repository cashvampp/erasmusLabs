CC = gcc
CFLAGS = -Wall -Wextra -pthread
LDFLAGS = -lssl -lcrypto

all: server

server: server.c
	$(CC) $(CFLAGS) -o server server.c $(LDFLAGS)

clean:
	rm -f server

.PHONY: all clean
