CC = clang # gcc
ARGS = -Wall -pedantic

.PHONY: release debug

debug: scheduler
	valgrind -q --leak-check=full ./$< -d

release: scheduler
	./$<

scheduler: scheduler.c
	$(CC) $(ARGS) -o3 $< -o scheduler


