/**
 * password_cracker
 * CS 241 - Spring 2021
 */
#include "cracker1.h"
#include "format.h"
#include "utils.h"
#include "includes/queue.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>       
#include <crypt.h>

static queue* q;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
static int success;

int test(char* pwd, char* hash, size_t idx, size_t len, int* numhash, struct crypt_data* cdata);
void* worker (void* number) {
    int threadid = *((int*)number);
    struct crypt_data cdata;
    cdata.initialized = 0;
    while(1) {
        char* command = queue_pull(q);
        // pthread_mutex_lock(&mutex);
        if (strcmp(command, "end") == 0) {
            queue_push(q, command);
            pthread_exit(NULL);
        }
        char user[9];
        char hash[14];
        char pwd[9];
        user[8] = '\0';
        hash[13] = '\0';
        pwd[8] = '\0';
        sscanf(command, "%s %s %s", user, hash, pwd);
        v1_print_thread_start(threadid, user);
        // pthread_mutex_unlock(&mutex);
        size_t idx = getPrefixLength(pwd);
        // for (size_t i = 0; i < 8; i++) {
        //     if (pwd[i] == '.') {
        //         idx = i;
        //         break;
        //     }
        // }
        int * numhash = malloc(sizeof(int));
        *numhash = 0;
        size_t len = strlen(pwd);
        int result = test(pwd, hash, idx, len, numhash, &cdata);
        double timeElapse = getThreadCPUTime();
        pthread_mutex_lock(&mutex);
        success += 1 - result;
        pthread_mutex_unlock(&mutex);
        v1_print_thread_result(threadid, user, pwd, *numhash, timeElapse, result);
        free(numhash);
        free(command);
    }
}
int test(char* pwd, char* hash, size_t idx, size_t len, int* numhash, struct crypt_data* cdata) {
    if (idx == len) {
        // printf(pwd);
        *numhash = (*numhash) + 1;
        char* hashed = crypt_r(pwd, "xx", cdata);
        if (strcmp(hashed, hash) == 0) {
            return 0;
        } else {
            return 1;
        }
    }
    for (char c = 'a'; c <= 'z'; c++) {
        pwd[idx] = c;
        int find = test(pwd, hash, idx + 1, len, numhash, cdata);
        if (find == 0) {
            return 0;
        }
    }
    return 1;
}
int start(size_t thread_count) {
    // TODO your code here, make sure to use thread_count!
    // Remember to ONLY crack passwords in other threads
    q = queue_create(-1);
    //create work thread;
    pthread_t threads[thread_count];
    int * idx = malloc(sizeof(int)*thread_count);
    for (size_t i = 0; i < thread_count; i++) {
        // pthread_mutex_lock(&mutex);
        idx[i] = i + 1;
        pthread_create(threads + i, NULL, worker, idx+i);
        // pthread_mutex_unlock(&mutex);
    }
    int total = 0;
    char* input = NULL;
    size_t inputsize = 0;
    while (1) {
        int inputlen = getline(&input,&inputsize,stdin); 
        if (inputlen == -1) {
            //waiting thread to finish.
            char* end = strdup("end");
            queue_push(q, end);
            break;
        }
        total ++;
        char * str = strdup(input);
        queue_push(q, str);
    }
    for (size_t i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
    char * end = queue_pull(q);
    free(input);
    free(end);
    free(idx);
    queue_destroy(q);
    v1_print_summary(success, total - success);
    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}
