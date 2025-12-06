# Database inspection utility Makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -O2
LDFLAGS = -L../../../libmdbx -lmdbx

# Include paths
INCLUDES = -I../../../libmdbx

# Source files
SOURCES = db_client.c

# Target
TARGET = db_client

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(SOURCES) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: clean