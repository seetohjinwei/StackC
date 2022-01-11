.PHONY: run_script

all: forth.out run_script

run_script:
	./forth.out foo.fth

forth.out: forth.c
	gcc -Wall forth.c -o forth.out

clean:
	rm forth.out

