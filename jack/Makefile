LDFLAGS=-ljack -lpthread
CC=gcc

all : vs_send vs_send2

vs_send : vs_send.o
	$(CC) $< -o $@ $(LDFLAGS)

vs_send2 : vs_send2.o
	$(CC) $< -o $@ $(LDFLAGS)
 
