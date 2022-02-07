#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

// debug flag
int debug = 0;

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
	self->array = (task_t*) calloc(cap, sizeof(task_t));
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

// Earliest due date algorithm using quicksort
void edd_quicksort(task_t* tasks, int32_t n) {
	qsort(tasks, n, sizeof(task_t), sort_edd);
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
			
			// this task is late 
			// - find the longest task in ot
			// - remove it from ot
			// - add it to lt

			int p_max = 0;
			for (int j = 0; ot.array[j].id != tasks[i].id; j++) {
				p_max = ot.array[j].p > ot.array[p_max].p ? j : p_max;
			}
			p_max = ot.array[i].p > ot.array[p_max].p ? i : p_max;

			vec_push(&lt, vec_delete_at(&ot, p_max)); 
	
			if (debug) {
				printf("OT[id: %i beeing late]----------------------------------------------\n", tasks[i].id);
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

// Compute the maximum profit
uint64_t compute_WI(task_t* tasks, uint32_t n) {
	uint32_t on_time = moore_hodgson(tasks, n);
	qsort(tasks, n, sizeof(task_t), sort_w_incr);
	
	uint64_t wi = 0;
	for (int i = (n - on_time - 1); i < n; i++) {
		wi += tasks[i].w;
	}

	qsort(tasks, n, sizeof(task_t), sort_id);
	edd_quicksort(tasks, n);
	return wi;
}

// debug the dynamic programming table
void debug_table(uint32_t** table, const size_t n, const size_t wi) {
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < (wi + 1); j++) {
			if (table[i][j] == UINT32_MAX) {
				printf("∞\t");
			} else {
				printf("%u\t", table[i][j]);
			}
		}
		puts("");
	}
}

// populate the dynamic programming table
void populate_table(uint32_t*** table, task_t* tasks, size_t n, size_t wi) {
	*table = (uint32_t**) calloc(n, sizeof(uint32_t*));
	uint32_t** p = *table;
	
	for (int i = 0; i < n; i++) {
		p[i] = (uint32_t*) calloc(wi + 1, sizeof(uint32_t));
	}
	

	for (int w = 1; w < (wi + 1); w++) {
		p[0][w] = UINT32_MAX;
	}
	p[0][tasks[0].w] = tasks[0].p;

	for (int i = 1; i < n; i++) {
		for (int w = 0; w < (wi + 1); w++) {

			int v = w - (int) tasks[i].w;
			
			uint32_t lhs = p[i-1][w];
			uint32_t rhs = v >= 0 ? p[i-1][v] : INT32_MAX;
			
			if (rhs != UINT32_MAX && rhs + tasks[i].p <= tasks[i].d) {
				p[i][w] = lhs >= rhs + tasks[i].p ? rhs + tasks[i].p : lhs;
			} else {
				p[i][w] = lhs;
			}
		}
	}
}

void find_sol(uint32_t** table, size_t n, uint64_t wi, task_t *tasks) {
	int j = n - 1;
	uint64_t profit = 0;
	uint64_t dd = tasks[n-1].d;

	vec_t solution;
	vec_init(&solution, n);

	for (int i = wi; i > 0; i--) {
		uint32_t v = table[j][i];
		if (v == UINT32_MAX) {
			continue;
		}
		if (dd == 0) {
			break;
		}

		if (j > 1 && dd == v && table[j-1][i] == UINT32_MAX) {
			vec_push(&solution, tasks[j]);
			
			dd -= tasks[j].p;
			profit += tasks[j].w;

			j -= 1;
		} else { 
			while (j >= 1 && dd == v && table[j-1][i] == v) {
				j -= 1;
			}
			vec_push(&solution, tasks[j]);
			dd -= tasks[j].p;
			
			profit += tasks[j].w;
		}
	}

	puts("----- Solution:");
	printf("Profit: %lu \n", profit);
	display_data(solution.array, solution.at, 0);

	vec_free(&solution);
}


void free_table(uint32_t** table, size_t n) {
	for (int i = 0; i < n; i++) {
		free(table[i]);
	}
	free(table);
}

int main(int argc, char** argv) {	
	uint32_t n = 0;
	uint64_t w_max = 0;
	task_t* tasks = NULL;

	load_data("i6.dat", &n, &tasks, &w_max);
	assert(n != 0);
	assert(w_max != 0);
	assert(tasks != NULL);

	edd_quicksort(tasks, n);

	debug = getopt(argc, argv, "d") != -1;
	if (debug) {
		display_data(tasks, n, w_max);
	}
	
	uint64_t wi = compute_WI(tasks, n);

	if (debug) {
		printf("WI: %lu\n", wi);
	}

	uint32_t** table = NULL;
	populate_table(&table, tasks, n, wi);
	assert(table != NULL);

	if (debug) {
		debug_table(table, n, wi);
	}
	
	find_sol(table, n, wi, tasks);

	free_table(table, n);
	free(tasks);
	return 0;
}
