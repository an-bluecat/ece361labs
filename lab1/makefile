

# all: ${TARGET}
# server: server.c


CFLAGS  = -g -Wall
CC = gcc
TARGET = deliver server

all: $(TARGET)

server: server.c
	$(CC) server.c $(CFLAGS) -o server 
deliver: deliver.c
	$(CC) deliver.c $(CFLAGS) -o deliver

clean:
	$(RM) $(TARGET)