CC = gcc
CFLAGS = -Wall -Wextra
LIBS = -larchive

CrarK: main.c
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)
