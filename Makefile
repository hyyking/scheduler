CC = gcc -Wall -pedantic

.PHONY: release debug

debug: scheduler
	./$< -d

release: scheduler
	./$<

scheduler: scheduler.c
	$(CC) -o3 $< -o scheduler


