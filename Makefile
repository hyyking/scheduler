CC = clang # gcc
ARGS = -Wall -pedantic
CLI = 

.PHONY: release-sched debug-sched release-bisched debug-bisched

debug-sched: scheduler
	valgrind -q --leak-check=full ./$< -d $(CLI)

release-sched: scheduler
	./$< $(CLI)

debug-bisched: bischeduler
	valgrind -q --leak-check=full ./$< -d $(CLI)

release-bisched: bischeduler
	./$< $(CLI)

scheduler: scheduler.c scheduler.h
	$(CC) $(ARGS) -o3 $< -o $@


bischeduler: bischeduler.c scheduler.h
	$(CC) $(ARGS) -o3 $< -o $@
