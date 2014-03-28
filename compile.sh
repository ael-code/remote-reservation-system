#!/bin/bash

options="-I lib -O3 -g -Wall -D_GNU_SOURCE " #-D DEBUG"

echo "compiling client"
gcc -o bin/client.o client/client.c lib/seats.c lib/conversion.c $options

echo "compiling server"
gcc -o bin/server.o server/server.c server/file_op.c server/threads.c server/reservation.c server/chiavazione.c server/matrix.c lib/seats.c lib/conversion.c -pthread $options
