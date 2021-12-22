#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

// debug flag
int debug = 0;

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

typedef struct {
	task_t* array;
	size_t at;
	size_t capacity;
} vec_t;

void display_data(task_t* tasks, uint32_t n, uint64_t w_max) {
	printf("Number of tasks: %d\n", n);
	printf("Max weight task: %li\n", w_max);
	
	for (int i = 0; i < n; i++) {
		printf("Task id: %d\n", tasks[i].id);
		printf("\tp: %i\n", tasks[i].p);
		printf("\tw: %i\n", (int) tasks[i].w);
		printf("\td: %li\n", (long) tasks[i].d);
	}
}

void vec_init(vec_t* self, size_t cap) {
	self->array = (task_t*) malloc(cap * sizeof(task_t));
	self->at = 0;
	self->capacity = cap;
}

void vec_copy_from(vec_t* self, task_t* tasks, size_t n) {
	assert(self->capacity >= n);
	memcpy(self->array, tasks, sizeof(task_t) * n);
	self->at = n;
}

task_t vec_delete_at(vec_t* self, size_t at) {
	assert(at <= self->at);
	
	task_t res = self->array[at];
	if (self->at == at) {
		self->at--;
		return res;
	}
	memmove(&self->array[at], &(self->array[at+1]), sizeof(task_t) * (self->at - at - 1));
	self->at--;
	return res;
}

task_t vec_delete_by_id(vec_t* self, uint32_t id) {
	for (int i = 0; i < self->at; i++)
		if (self->array[i].id == id)
			return vec_delete_at(self, i);

	puts("element not present in vec, vec_delete_by_id");
	exit(1);
}

void vec_push(vec_t* self, task_t task) {
	assert(self->at <= self->capacity);
	self->array[self->at++] = task;
}

void vec_free(vec_t* self) {
	free(self->array);
}

// Swap the position of two tasks in the array
void swap(task_t* tasks, uint32_t left, uint32_t right) {
	task_t temp = tasks[left];
	tasks[left] = tasks[right];
	tasks[right] = temp;
}

// Find the partition point for a given pivot comparing by duration
uint32_t partition_d(task_t* tasks, int32_t left, int32_t right, uint32_t pivot) {
	int l = left - 1;
	int r = right;

	for (;;) {
		while (tasks[++l].d < pivot) {}
		while (r > 0 && tasks[--r].d > pivot) {}
		if (l >= r) {
			break;
		} else {
			swap(tasks, l, r);
		}
	}
	swap(tasks, l, right);
	return l;
}

// Earliest due date algorithm using quicksort
void edd_quicksort(task_t* tasks, int32_t left, int32_t right) {
	if (right - left <= 0) return;
	
	uint32_t pivot = tasks[right].d;
	int32_t part  = partition_d(tasks, left, right, pivot);
	edd_quicksort(tasks, left, part - 1);
	edd_quicksort(tasks, part + 1, right);
}

// Load an input data file allocating and filling an array of task_t
void load_data(const char filename[], uint32_t* n, task_t** tasks, uint64_t* w_max) {
	FILE* f = fopen(filename, "r");
	assert(f != NULL);

	assert(fscanf(f, "%u", n) != EOF);

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

// Moore Hodgson Algorithm for computing the number of on-time tasks
uint32_t moore_hodgson(task_t* tasks, uint32_t n) {
	vec_t ot;
	vec_init(&ot, n);
	vec_copy_from(&ot, tasks, n);
	
	vec_t lt;
	vec_init(&lt, n);
	
	uint32_t curr_p = 0;
	for (int i = 0; i < n; i++) {
		if (ot.at == 0) return 0;

		curr_p += tasks[i].p;
		if (curr_p > tasks[i].d) {
			curr_p -= tasks[i].p;
			
			// this task is late, add it 
			// - find the longest task in lo until id is found
			// - remove it from lo
			// - add it to lt
			
			int p_max = 0;
			for (int j = 0; ot.array[j].id != tasks[i].id; j++) {
				p_max = ot.array[j].p > ot.array[p_max].p ? j : p_max;
			}
			p_max = ot.array[i].p > ot.array[p_max].p ? i : p_max;

			vec_push(&lt, vec_delete_at(&ot, p_max)); 
	
			if (debug) {
				printf("LO[id: %i beeing late]----------------------------------------------\n", tasks[i].id);
				display_data(ot.array, ot.at, 0);
				printf("LT[id: %i beeing late]----------------------------------------------\n", tasks[i].id);
				display_data(lt.array, lt.at, 0);
			}
		}
	}
	
	uint32_t ot_n = ot.at;
	vec_free(&ot);
	vec_free(&lt);
	return ot_n;
}

int main(int argc, char** argv) {	
	uint32_t n = 0;
	uint64_t w_max = 0;
	task_t* tasks = NULL;

	load_data("i6.dat", &n, &tasks, &w_max);
	assert(n != 0);
	assert(w_max != 0);
	assert(tasks != NULL);

	edd_quicksort(tasks, 0, n - 1);

	debug = getopt(argc, argv, "d") != -1;
	if (debug) {
		display_data(tasks, n, w_max);
	}
	
	printf("Moore: %i", moore_hodgson(tasks, n));


	free(tasks);
	return 0;
}
