/**
 * critical_concurrency
 * CS 241 - Spring 2021
 */
#include "barrier.h"
#include <unistd.h>


// The returns are just for errors if you want to check for them.
int barrier_destroy(barrier_t *barrier) {
    int error = 0;
    pthread_mutex_destroy(&barrier->mtx);
    pthread_cond_destroy(&barrier->cv);
    return error;
}

int barrier_init(barrier_t *barrier, unsigned int num_threads) {
    int error = 0;
    pthread_mutex_init(&barrier->mtx, NULL);
    pthread_cond_init(&barrier->cv, NULL);
    barrier->n_threads = num_threads;
    barrier->count = 0;
    barrier->times_used = 0;
    return error;
}

int barrier_wait(barrier_t *barrier) {
    pthread_mutex_lock(&barrier->mtx);
    if (barrier->count == barrier->n_threads) {
        if (barrier->times_used == barrier->n_threads) {
            barrier->count = 0;
            pthread_cond_broadcast(&barrier->cv);
        }
        while(barrier->times_used < barrier->n_threads) {
            pthread_cond_wait(&barrier->cv, &barrier->mtx);
        }
    }
    barrier->count++;
    if (barrier->count == barrier->n_threads) {
        pthread_cond_broadcast(&barrier->cv);
        barrier->times_used = 0;
    }
    while(barrier->count < barrier->n_threads) {
        pthread_cond_wait(&barrier->cv, &barrier->mtx);
    }
    barrier->times_used++;
    pthread_mutex_unlock(&barrier->mtx);
    return 0;
}
