.PHONY: run_script

all: forth.out run_script

run_script:
	python3 test.py

forth.out: forth.c
	gcc -Wall forth.c -o forth.out

clean:
	rm forth.out

