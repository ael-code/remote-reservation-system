#!/bin/bash

echo "compiling client"
gcc -o client/client.out client/client.c lib/seats.c lib/conversion.c  -O3

echo "compiling server"
gcc -o server/server.out server/server.c server/reservation.c server/chiavazione.c server/matrix.c lib/seats.c lib/conversion.c -pthread -O3
