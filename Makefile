CC = clang # gcc
ARGS = -Wall -pedantic

.PHONY: release debug

debug: scheduler
	valgrind -q ./$< -d

release: scheduler
	./$<

scheduler: scheduler.c
	$(CC) $(ARGS) -o3 $< -o scheduler


