#!/bin/sh

CURL="-lcurl -I/usr/include/curl/"
CFLAGS="-Wall -Werror -g -std=c11 -D_XOPEN_SOURCE=700"
LDFLAGS="$CURL"

gcc token.c -c -o token.o $CFLAGS
gcc torrent.c -c -o torrent.o $CFLAGS
gcc main.c -c -o main.o $CFLAGS
gcc main.o token.o torrent.o -o main $LDFLAGS $CFLAGS
