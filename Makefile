CC = gcc
CFLAGS = -pthread -g -Werror -O2

SRCDIR = src

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(SRCS:.c=.o)
TARGET = ws.out

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean

clean:
	rm -f $(OBJS) $(TARGET)
