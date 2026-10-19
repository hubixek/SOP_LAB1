CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=gnu11 -g -O2
TARGET = zadanko1

all: $(TARGET)

$(TARGET): zadanko1.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)

.PHONY: all clean