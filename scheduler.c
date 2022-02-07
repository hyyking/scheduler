#include "scheduler.h"

// debug flag
int debug = 0;


// Moore Hodgson Algorithm for computing the number of on-time tasks
uint32_t moore_hodgson(task_t* tasks, uint32_t n) {
    vec_t ot;
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
                printf("OT[id: %i beeing late]------------------\n", tasks[i].id);
                display_data(ot.array, ot.at, 0);
                printf("LT[id: %i beeing late]------------------\n", tasks[i].id);
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
				p[i][w] = MIN(lhs, rhs + tasks[i].p);
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

float64_t compute_epsilon(double k, size_t n, uint64_t w_max)
{
    return (k / (w_max / n));
}

static void     redefine_weight(unsigned int n, TASK *tasks, double k)
{
    unsigned int    index = 0;

    for (index = 0; tasks != NULL && tasks[index] != NULL && index < n; index += 1) {
        tasks[index]->w /= k;
    }
}

void fully_polynomial_time_approximation_scheme(double k, unsigned int n, TASK *tasks, unsigned long long w_max)
{
    double epsilon = compute_epsilon(k, n, w_max);

    printf("ε = %.3lf\n", epsilon);
    redefine_weight(n, tasks, k);
}


int main(int argc, char** argv) {	
	uint32_t n = 0;
	uint64_t w_max = 0;
	task_t* tasks = NULL;

        if (argc < 2) {
            fprintf(stderr, "Missing file argument");
            exit(1);
        }

	load_data(argv[argc - 1], &n, &tasks, &w_max);
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
