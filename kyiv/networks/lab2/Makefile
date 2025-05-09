CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic

all: server client

udp_server: server.c
	$(CC) $(CFLAGS) -o server server.c

udp_client: client.c
	$(CC) $(CFLAGS) -o client client.c

clean:
	rm -f server client

.PHONY: all clean
