#!/bin/bash

options="-I lib -O3 -g -Wall"

echo "compiling client"
gcc -o client/client.out client/client.c lib/seats.c lib/conversion.c $options

echo "compiling server"
gcc -o server/server.out server/server.c server/file_op.c server/threads.c server/reservation.c server/chiavazione.c server/matrix.c lib/seats.c lib/conversion.c -pthread $options
