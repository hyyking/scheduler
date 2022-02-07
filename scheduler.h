#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

#define MIN(a,b)    (((a) < (b)) ? (a) : (b))
#define MAX(a,b)    (((a) > (b)) ? (a) : (b))

typedef struct {
	// id: An unsigned integer for storing the index of the task (T 1 is the first task, its id is one)
	uint32_t id;
	// p: An unsigned integer for storing the processing time of the task
	uint32_t p;
	// w: An unsigned long long for storing the weight of the task
	uint64_t w;
	// d: An unsigned integer d for storing the due date of the task
	uint32_t d;
} task_t;


typedef struct {
	task_t* array;
	size_t at;
	size_t capacity;
} vec_t;

void display_task(task_t task) {
    printf("Task id: %d\n", task.id);
    printf("\tp: %i\n", task.p);
    printf("\tw: %i\n", (int) task.w);
    printf("\td: %li\n", (long) task.d);
}

void display_data(task_t* tasks, uint32_t n, uint64_t w_max) {
	printf("Number of tasks: %d\n", n);
	printf("Max weight task: %li\n", w_max);
	
	for (int i = 0; i < n; i++) {
            display_task(tasks[i]);
	}
}

void vec_init(vec_t* self, size_t cap) {
	self->array = (task_t*) calloc(cap, sizeof(task_t));
	self->at = 0;
	self->capacity = cap;
}

void vec_copy_from(vec_t* self, task_t* tasks, size_t n) {
        vec_init(self, n);
	self->at = n;
	self->capacity = n;
	memcpy(self->array, tasks, sizeof(task_t) * n);
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

void vec_push_front(vec_t* self, task_t task) {
	assert(self->at + 1 <= self->capacity);
	memmove(&(self->array[1]), &(self->array[0]), sizeof(task_t) * self->at);
        self->array[0] = task;
        self->at++;
}

void vec_free(vec_t* self) {
	free(self->array);
}

int sort_w_incr(const void* a, const void* b) {
	task_t* lhs = (task_t*) a;
	task_t* rhs = (task_t*) b;
	return rhs->w < lhs->w;
}

int sort_id(const void* a, const void* b) {
	task_t* lhs = (task_t*) a;
	task_t* rhs = (task_t*) b;
	return rhs->id < lhs->id;
}

int sort_edd(const void* a, const void* b) {
	task_t* lhs = (task_t*) a;
	task_t* rhs = (task_t*) b;
	return rhs->d < lhs->d;
}

int sort_spp(const void* a, const void* b) {
	task_t* lhs = (task_t*) a;
	task_t* rhs = (task_t*) b;
	return rhs->p < lhs->p;
}

// Earliest due date algorithm using quicksort
void edd_quicksort(task_t* tasks, size_t n) {
	qsort(tasks, n, sizeof(task_t), sort_edd);
}

// Shortest processing time quicksort
void spt_quicksort(task_t* tasks, size_t n) {
	qsort(tasks, n, sizeof(task_t), sort_spp);
}

// Sum processing time
uint32_t sum_proctime(task_t* tasks, size_t n) {
    uint32_t accu = 0;
    for (int i = 0; i < n; i++) accu += tasks[i].p;
    return accu;
}

// Sum completion time
uint32_t sum_comptime(vec_t tasks) {
    uint32_t accu = 0;
    uint32_t sum_ct = 0;
    for (int i = 0; i < tasks.at; i++) {
        accu += tasks.array[i].p;
        sum_ct += accu;
    }
    return sum_ct;
}

// Load an input data file allocating and filling an array of task_t
void load_data(const char filename[], uint32_t* n, task_t** tasks, uint64_t* w_max) {
	FILE* f = fopen(filename, "r");
	assert(f != NULL);

	assert(fscanf(f, "%u", n) != EOF);

	// assign the array pointer to valid memory
	*tasks = (task_t*) calloc(*n, sizeof(task_t));
	task_t* t = *tasks;
	
	*w_max = 0;
	for (int i = 0; i < *n; i++) {
		assert(fscanf(f, "%li %li %lli", (long*) &t[i].p, (long*) &t[i].w, (long long*) &t[i].d) != EOF);
		*w_max = MAX(t[i].w, *w_max);


		t[i].id = i + 1;
	}

	fclose(f);
}
