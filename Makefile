.PHONY: run_script

all: forth run_script

run_script:
	python3 test.py

forth: forth.c
	gcc -Wall forth.c -o forth

clean:
	rm forth

