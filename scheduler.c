#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>


// id: An unsigned integer for storing the index of the task (T 1 is the first task, its id is one)
// p: An unsigned integer for storing the processing time of the task
// w: An unsigned long long for storing the weight of the task
// d: An unsigned integer d for storing the due date of the task
typedef struct {
	uint32_t id;
	uint32_t p;
	uint64_t w;
	uint32_t d;
} task_t;

void swap(task_t* tasks, uint32_t left, uint32_t right) {
	task_t temp = tasks[left];
	tasks[left] = tasks[right];
	tasks[right] = temp;
}

uint32_t partition_p(task_t* tasks, int32_t left, int32_t right, uint32_t pivot) {
	int l = left - 1;
	int r = right;

	for (;;) {
		while (tasks[++l].d < pivot) {}
		while (r > 0 && tasks[--r].d > pivot) {}
		if (l >= r) {
			break;
		} else {
			// swap(tasks, l, r);
			task_t temp = tasks[l];
			tasks[l] = tasks[r];
			tasks[r] = temp;
		}
	}
	// swap(tasks, l, right);
	task_t temp = tasks[l];
	tasks[l] = tasks[right];
	tasks[right] = temp;
	return l;
}

void quicksort_p(task_t* tasks, int32_t left, int32_t right) {
	if (right - left <= 0) {
		return;
	}
	uint32_t pivot = tasks[right].d;
	int32_t part  = partition_p(tasks, left, right, pivot);
	quicksort_p(tasks, left, part - 1);
	quicksort_p(tasks, part + 1, right);
}

void load_data(const char filename[], uint32_t* n, task_t** tasks, uint64_t* w_max) {
	FILE* f = fopen(filename, "r");
	assert(f != NULL);

	assert(fscanf(f, "%i", (int*) n) != EOF);

	// assign the array pointer to valid memory
	*tasks = (task_t*) malloc((unsigned long) *n * sizeof(task_t));
	task_t* t = *tasks;
	
	*w_max = 0;
	for (int i = 0; i < *n; i++) {
		assert(fscanf(f, "%li %li %lli", (long*) &t[i].p, (long*) &t[i].w, (long long*) &t[i].d) != EOF);
		*w_max = t[i].w > *w_max ? t[i].w : *w_max;
		t[i].id = i + 1;
	}

	fclose(f);
}

void display_data(task_t* tasks, uint32_t n, uint64_t w_max) {
	printf("Number of tasks: %d\n", n);
	printf("Max weight task: %d\n", n);
	
	for (int i = 0; i < n; i++) {
		printf("Task id: %d\n", tasks[i].id);
		printf("\tp: %i\n", tasks[i].p);
		printf("\tw: %i\n", (int) tasks[i].w);
		printf("\td: %li\n", (long) tasks[i].d);
	}
}

int main(int argc, char** argv) {	
	uint32_t n = 0;
	uint64_t w_max = 0;
	task_t* tasks = NULL;

	load_data("i6.dat", &n, &tasks, &w_max);
	assert(n != 0);
	assert(w_max != 0);
	assert(tasks != NULL);
	quicksort_p(tasks, 0, n - 1);

	if (getopt(argc, argv, "d") != -1) {
		display_data(tasks, n, w_max);
	}


	free(tasks);

	return 0;
}
