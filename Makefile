solver.exe: main.o
	gcc -o solver.exe main.c

main.o: main.c
	gcc -c main.c
