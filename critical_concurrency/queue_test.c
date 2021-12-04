/**
 * critical_concurrency
 * CS 241 - Spring 2021
 */
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "queue.h"
<<<<<<< HEAD
static pthread_t pthreads[2];
void* thread(void* queue){
    for (int i = 0; i < 10; i++) {
        int a = 1;
        queue_push(queue, &a);
        int* b = queue_pull(queue);
        queue_push(queue, &b);
    }
    return NULL;
}
int main(int argc, char **argv) {
    queue* q = queue_create(10);
    for (int i = 0; i < 2; i++) {
        pthread_create(&pthreads[i], NULL, &thread, q);
    }
    for (int i = 0; i < 2; i++) {
        pthread_join(pthreads[i], NULL);
    }
    queue_destroy(q);
=======

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("usage: %s test_number return_code\n", argv[0]);
        exit(1);
    }
    printf("Please write tests cases\n");
>>>>>>> 4e017b983cba416b7655ea6088cd2c9f114d5997
    return 0;
}
