#include "scheduler.h"
#include <sys/timeb.h>

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
                printf("OT[id: %i beeing late]------------------\n", tasks[p_max].id);
                display_data(ot.array, ot.at, 0);
                printf("LT[id: %i beeing late]------------------\n", tasks[p_max].id);
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
    
    if (debug) {
        printf("On time tasks: %u\n", on_time);
    }

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
            uint32_t rhs = v >= 0 ? p[i-1][v] : UINT32_MAX;
                
            if (rhs != UINT32_MAX && rhs + tasks[i].p <= tasks[i].d) {
                p[i][w] = MIN(lhs, rhs + tasks[i].p);
            } else {
                p[i][w] = lhs;
            }
        }
    }
}

void find_sol(uint32_t** table, size_t n, uint64_t wi, task_t *tasks, double k) {
    edd_quicksort(tasks, n);

    int j = n - 1;
    uint64_t profit = 0;
    int dd = tasks[n-1].d;

    for (int i = wi; i > 0; i--) {
        if (table[j][i] != UINT32_MAX) {
            dd = table[j][i];
            break;
        }
    }

    vec_t solution;
    vec_init(&solution, 3 * n);
    
    for (int i = wi; i > 0; i--) {
        uint32_t v = table[j][i];
        if (v == UINT32_MAX || dd < v) {
                continue;
        }
        if (dd == 0) {
                break;
        }
        
        if (j > 1 && dd == v && table[j-1][i] == UINT32_MAX) {
            vec_push(&solution, tasks[j]);
            
            dd -= tasks[j].p;
            profit += tasks[j].w;

            j--;
        } else if (j >= 1 && dd == v) { 
            while (table[j-1][i] == v) {
                j--;
            }
            vec_push(&solution, tasks[j]);

            dd -= tasks[j].p;
            profit += tasks[j].w;
            j--;
        }
    }

    puts("----- Solution:");
    printf("Profit: %.3lf \n", profit * k);
    display_data(solution.array, solution.at, 0);

    vec_free(&solution);
}


void free_table(uint32_t** table, size_t n) {
	for (int i = 0; i < n; i++) {
		free(table[i]);
	}
	free(table);
}

void replace_weights(task_t* tasks, size_t n, double k)
{
    for (int i = 0; i < n; i++) {
        tasks[i].w = tasks[i].w / k;
    }
}

void solve(task_t* tasks, size_t n, double k) {
    uint64_t wi = compute_WI(tasks, n);

    if (debug) {
        printf("WI: %lu\n", wi);
    }

    uint32_t** table = NULL;
    populate_table(&table, tasks, n, wi);
    assert(table != NULL);

    if (debug && n < 20) {
        debug_table(table, n, wi);
    }
    
    find_sol(table, n, wi, tasks, k);

    free_table(table, n);
}


int main(int argc, char** argv) {	
	uint32_t n = 0;
	uint64_t w_max = 0;
	task_t* tasks = NULL;

        if (argc < 2) {
            fprintf(stderr, "Missing file argument");
            exit(1);
        }
	debug = getopt(argc, argv, "d") != -1;
        
        if (argc == 2 + debug) {
            load_data(argv[argc - 1], &n, &tasks, &w_max);
        }

        double k = 1;
        if (argc == 3 + debug) {
	    load_data(argv[argc - 2], &n, &tasks, &w_max);

	    k = atof(argv[argc - 1]);
            double epsilon = (k / (w_max / n));
            printf("epsilon: %.5lf\n", epsilon);
            replace_weights(tasks, n, k);
        }

	assert(n != 0);
	assert(w_max != 0);
	assert(tasks != NULL);

	edd_quicksort(tasks, n);

	if (debug) {
	    display_data(tasks, n, w_max);
	}


        struct timeb t0, t1;
        ftime(&t0);
        solve(tasks, n, k);
        ftime(&t1);
        float cpu_time = (float)(t1.time - t0.time) + (float)(t1.millitm-t0.millitm)/1000;
        printf("\n[CPU Time] %.3f s.\n", cpu_time);

	free(tasks);
	return 0;
}
