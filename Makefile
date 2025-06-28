# Makefile for the Wavelet Filter project

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I.
LDFLAGS = -lm

# Source files
SRCS = main.c wavelet_filter.c
TEST_SRCS = test_wavelet_filter.c wavelet_filter.c

# Object files
OBJS = $(SRCS:.c=.o)
TEST_OBJS = $(TEST_SRCS:.c=.o)

# Executables
TARGET = main
TEST_TARGET = test_wavelet_filter

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(TEST_TARGET): $(TEST_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TEST_OBJS) $(TARGET) $(TEST_TARGET)