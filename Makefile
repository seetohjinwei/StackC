CC = gcc
CFLAGS = -Wall
CFLAGS_FULL = -Wall -Wextra -pedantic
.PHONY: run_tests

default: run_tests

verbose: stackc test
	./test -dv tests

update: stackc test
	./test -du tests

run_tests: stackc test
	./test -d tests

stackc: stackc.c
	$(CC) $(CFLAGS) -o stackc stackc.c

test: test.c
	$(CC) $(CFLAGS_FULL) -o test test.c

clean:
	rm stackc test
