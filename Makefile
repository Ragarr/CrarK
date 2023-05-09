CC = gcc
CFLAGS = -Wall -Wextra 
LIBS = -larchive -lm 

CrarK: main.c
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)
