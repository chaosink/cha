CFLAGS= -Wall -Wextra
CC=gcc

all: client-local client groupwork

groupwork: server.c
	gcc -Wall -Wextra -o groupwork server.c
