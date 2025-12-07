# Database inspection utility Makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -O2

# libmdbx library paths (matching parent project configuration)
MDBX_INCLUDES = -I/Users/ahmed/dev/libmdbx/build-local/include
MDBX_LDFLAGS = /Users/ahmed/dev/libmdbx/build-local/lib/libmdbx.a

# Source files
SOURCES = db_client.c

# Target
TARGET = db_client

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(MDBX_INCLUDES) -o $(TARGET) $(SOURCES) $(MDBX_LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: clean