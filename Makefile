.PHONY: run_script

all: stackc run_script

run_script:
	./test.py

stackc: stackc.c
	gcc -Wall stackc.c -o stackc

clean:
	rm stackc
