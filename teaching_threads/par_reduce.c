/**
 * teaching_threads
 * CS 241 - Spring 2021
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "reduce.h"
#include "reducers.h"

/* You might need a struct for each task ... */
typedef struct task {
    int base_case;
    int* list;
    size_t length;
    reducer reduce_func;
} task; 
/* You should create a start routine for your threads. */
void* startR(void* tasks) {
    task* task_i = (task*) tasks;
    int result = task_i->base_case;

    for (size_t i = 0; i < task_i->length; ++i) {
        result = task_i->reduce_func(result, task_i->list[i]);
    }
    int* toreturn = malloc(sizeof(int));
    *toreturn = result;
    return toreturn;
}

int par_reduce(int *list, size_t list_len, reducer reduce_func, int base_case,
               size_t num_threads) {
    /* Your implementation goes here */
    if (list_len < num_threads) {
        int result = base_case;
        for (size_t i = 0; i < list_len; ++i) {
            result = reduce_func(result, list[i]);
        }
        return result;
    }
    size_t thread_len[num_threads];
    for (size_t i = 0; i < num_threads; i++) {
        thread_len[i] = list_len/num_threads;
    }
    for (size_t i = 0; i < list_len % num_threads; i++) {
        thread_len[i] += 1;
    }
    size_t count = 0;
    int result[num_threads];
    pthread_t threads[num_threads];
    task* tasks[num_threads];
    for (size_t i = 0; i < num_threads; i++) {
        tasks[i] = (task*) malloc(sizeof(task));
        tasks[i] ->base_case = base_case;
        tasks[i]->list = list + count;
        count += thread_len[i];
        tasks[i]->length = thread_len[i];
        tasks[i]->reduce_func = reduce_func;
        pthread_create(threads + i, NULL, startR, tasks[i]);
    }
    for (size_t i = 0; i < num_threads; i++) {
        void* resulti;
        pthread_join(threads[i], &resulti);
        result[i] = *(int*)resulti;
        free(tasks[i]);
        free(resulti);
    }
    int finalresult = base_case;
    for (size_t i = 0; i < num_threads; ++i) {
        finalresult = reduce_func(finalresult, result[i]);
    }
    return finalresult;
}
