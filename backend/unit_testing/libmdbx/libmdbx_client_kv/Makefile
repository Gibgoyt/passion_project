CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99 -I/home/opc/splitdo/new/libmdbx
LDFLAGS = -L/home/opc/splitdo/new/libmdbx/build -lmdbx -Wl,-rpath,/home/opc/splitdo/new/libmdbx/build

all: server client

server: server.c
	$(CC) $(CFLAGS) -o server server.c $(LDFLAGS)

client: client.c
	$(CC) $(CFLAGS) -o client client.c $(LDFLAGS)

clean:
	rm -f server client
	rm -rf data/

clean-data:
	rm -rf data/

run: server
	./server

test: all
	@echo "Starting test..."
	@./test_script.sh || echo "Run 'make test' after starting server in another terminal"

install-deps:
	@echo "Installing libmdbx if needed..."
	@which yay >/dev/null && yay -S --needed libmdbx || echo "Please install libmdbx manually"

.PHONY: all clean clean-data run test install-deps