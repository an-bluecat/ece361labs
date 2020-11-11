

# all: ${TARGET}
# server: server.c


CFLAGS  = -g -Wall
CC = gcc
TARGET = client server

all: $(TARGET)

server: server.c
	$(CC) server.c $(CFLAGS) -o server 
client: client.c
	$(CC) client.c $(CFLAGS) -o client

clean:
	$(RM) $(TARGET)