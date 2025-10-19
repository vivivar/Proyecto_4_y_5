main: main.c
	gcc main.c $(shell pkg-config --cflags --libs gtk+-3.0) -o TheSimplexSolver -export-dynamic

run: main
	./TheSimplexSolver

clean:
	rm -f main
