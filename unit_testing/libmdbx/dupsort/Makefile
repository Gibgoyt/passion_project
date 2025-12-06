CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99 -I/Users/ahmed/dev/libmdbx/build-local/include
LDFLAGS = /Users/ahmed/dev/libmdbx/build-local/lib/libmdbx.a

all: server client test_dupsort

server: server.c
	$(CC) $(CFLAGS) -o server server.c $(LDFLAGS)

client: client.c
	$(CC) $(CFLAGS) -o client client.c $(LDFLAGS)

test_dupsort: test_dupsort.c
	$(CC) $(CFLAGS) -o test_dupsort test_dupsort.c $(LDFLAGS)

clean:
	rm -f server client test_dupsort
	rm -rf data/
	rm -f test_dupsort_data

clean-data:
	rm -rf data/
	rm -f test_dupsort_data

run: server
	./server

run-unit: test_dupsort
	./test_dupsort

test: all
	@echo "Starting dupsort test..."
	@./test_script.sh || echo "Run 'make test' after starting server in another terminal"

install-deps:
	@echo "Installing libmdbx if needed..."
	@which yay >/dev/null && yay -S --needed libmdbx || echo "Please install libmdbx manually"

.PHONY: all clean clean-data run run-unit test install-deps
