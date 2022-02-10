CC = gcc
CFLAGS = -Wall
.PHONY: run_script

all: stackc run_script

run_script:
	./test.py

stackc: stackc.c
	$(CC) $(CFLAGS) stackc.c -o stackc

clean:
	rm stackc
