all: mathwait

mathwait: mathwait.c
	gcc -std=c99 -o mathwait mathwait.c
	

clean:
	rm -f mathwait
