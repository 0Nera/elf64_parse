.phony: parser test run all

all: parser test run

parser:
	gcc main.c -o elf_parce 

test:
	gcc -g -O0 test.c -o test.elf

run:
	./elf_parce test.elf
	./test.elf