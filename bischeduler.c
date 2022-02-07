#include "scheduler.h"

int debug = 0;

uint32_t update_delta(vec_t tasks, uint32_t delta, uint32_t L0, uint32_t sp, task_t max_task)
{
    task_t* Ti = NULL;

    for (int i = 0; i < tasks.at; i++) {
        task_t* task = &tasks.array[i];
        if (task->d + L0 < sp && task->p > max_task.p) {
            if (Ti == NULL || (sp - (task->d + L0)) < (sp - (Ti->d + L0))) {
                Ti = task;
            }
        }
    }

    return (Ti != NULL && delta > (sp - (Ti->d + L0))) ? sp - (Ti->d + L0) : delta;
}

// Minimise completion time
uint32_t min_comptime(vec_t* tasks, uint32_t L0)
{
    vec_t taskvec;
    vec_copy_from(&taskvec, tasks->array, tasks->at);

    uint32_t sp = sum_proctime(taskvec.array, taskvec.capacity);
    uint32_t delta = sp;

    tasks->at = 0; // clear the whole vector
    while (taskvec.at != 0) {
        // serach for the longest task
        task_t* max_task = NULL;
        for (int i = 0; i < taskvec.at; i++) {
            task_t* task = &taskvec.array[i];
            if (task->d + L0 >= sp && (max_task == NULL || task->p > max_task->p)) {
                max_task = task;
            }
        }

        vec_push_front(tasks, *max_task);
        delta = update_delta(taskvec, delta, L0, sp, *max_task);
        sp -= max_task->p;

        vec_delete_by_id(&taskvec, max_task->id);
    }

    vec_free(&taskvec);
    return (delta);
}

const uint32_t compute_Lmax(const vec_t tasks) {
    uint32_t lateness = 0;
    int64_t pt = 0;

    for (int i = 0; i < tasks.at; i++) {
        task_t task = tasks.array[i];
        lateness = MAX(lateness, pt + (int64_t) task.p - (int64_t) task.d);
        pt += task.p;
    }
    return lateness;
}


void solve(vec_t tasks) {

    edd_quicksort(tasks.array, tasks.at);
    uint32_t L0 = compute_Lmax(tasks);

    spt_quicksort(tasks.array, tasks.at);
    uint32_t L1 = compute_Lmax(tasks);

    size_t idx = 0;

    printf("y_%lu = (%u, %u)\n", idx++, sum_comptime(tasks), L1);
    while (L0 < L1) {
        L0 += min_comptime(&tasks, L0);
        printf("γ_%lu = (%u, %d)\n", idx++, sum_comptime(tasks), compute_Lmax(tasks));
    }
}


int main(int argc, char** argv) {	
	uint32_t n = 0;
	uint64_t w_max = 0;
	task_t* tasks = NULL;
        
        if (argc < 2) {
            puts("Missing file argument");
            exit(1);
        }
	load_data(argv[argc - 1], &n, &tasks, &w_max);
        	
        assert(n != 0);
	assert(w_max != 0);
	assert(tasks != NULL);

	debug = getopt(argc, argv, "d") != -1;
	if (debug) {
		display_data(tasks, n, w_max);
	}

	vec_t vtasks;
	vec_copy_from(&vtasks, tasks, n);       
	
        solve(vtasks);
	
        vec_free(&vtasks);
	free(tasks);
	return 0;
}
