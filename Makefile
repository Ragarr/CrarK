CC = gcc
CFLAGS = -Wall -Wextra
LIBS = -larchive -lm

SRCS = main.c password_utils.c
OBJS = $(SRCS:.c=.o)
TARGET = CrarK

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LIBS)
	rm -f $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	
