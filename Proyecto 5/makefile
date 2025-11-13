main: main.c simplex.c latex.c simplex.h latex.h
	gcc main.c simplex.c latex.c $(shell pkg-config --cflags --libs gtk+-3.0) -o TheSimplexSolver -export-dynamic -lm

run: main
	./TheSimplexSolver

clean:
	rm -f TheSimplexSolver *.tex *.aux *.log *.pdf *.out