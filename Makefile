CC = gcc -Wall -pedantic

.PHONY: release debug

debug: scheduler
	./a.out -d

release: scheduler
	./a.out

scheduler: scheduler.c
	$(CC) -lm $<


