all: compile run
compile:
	gcc -o program main.c
run:
	./program
