CC = gcc
DEBUG =
CFLAGS = -I lib -O3 -Wall -D_GNU_SOURCE -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

all: bin bin/client bin/server

bin:
	mkdir -p bin


## Client
bin/rss-client: bin/client.o bin/seats.o bin/conversion.o
	$(CC) -o bin/rss-client bin/client.o bin/seats.o bin/conversion.o $(LFLAGS)

bin/client.o: client/client.c
	$(CC) -o bin/client.o client/client.c $(CFLAGS)


## Server
bin/rss-server: bin/server.o bin/file_op.o bin/threads.o bin/reservation.o bin/chiavazione.o bin/matrix.o bin/seats.o bin/conversion.o
	$(CC) -o bin/rss-server bin/server.o bin/file_op.o bin/threads.o bin/reservation.o bin/chiavazione.o bin/matrix.o bin/seats.o bin/conversion.o $(LFLAGS) -pthread

bin/server.o: server/server.c
	$(CC) -o bin/server.o server/server.c $(CFLAGS)
	
bin/file_op.o: server/file_op.c
	$(CC) -o bin/file_op.o server/file_op.c $(CFLAGS)

bin/threads.o: server/threads.c
	$(CC) -o bin/threads.o server/threads.c $(CFLAGS)
	
bin/reservation.o: server/reservation.c
	$(CC) -o bin/reservation.o server/reservation.c $(CFLAGS)

bin/chiavazione.o: server/chiavazione.c
	$(CC) -o bin/chiavazione.o server/chiavazione.c $(CFLAGS)

bin/matrix.o: server/matrix.c
	$(CC) -o bin/matrix.o server/matrix.c $(CFLAGS)
	
	

## Library	
bin/seats.o: lib/seats.c
	$(CC) -o bin/seats.o lib/seats.c $(CFLAGS)
	
bin/conversion.o: lib/conversion.c
	$(CC) -o bin/conversion.o lib/conversion.c $(CFLAGS)



.PHONY: clean_bin clean_bk stat

clean_bin:
	rm ./bin/*
clean_bk:
	rm ./*.bk
stat:	
	./project_stat.sh
