/**
 * password_cracker
 * CS 241 - Spring 2021
 */
#include "cracker2.h"
#include "format.h"
#include "utils.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>       
#include <crypt.h>
#include <assert.h>
#include "includes/queue.h"
struct task {
    char* password;
    char* hash;
    char* user;
};
static pthread_cond_t cv;
static pthread_cond_t cv2;
// static queue* q;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
static size_t threadcount;

static int push;
static int eof;
static size_t finishthread;
static int found;
static char* passwordfound;
static struct task* info;
static int totalcount;
static size_t threadexit;

void* worker (void* number) {
    struct crypt_data cdata;
    cdata.initialized = 0;
    int threadid = *((int*)number);
    while(1) {
        // const char* command = queue_pull(q);
        pthread_mutex_lock(&mutex);
        while(push == 0) {
            pthread_cond_wait(&cv, &mutex);
        }
        push --;
        if (eof == 1) {
            // printf("end");
            pthread_mutex_unlock(&mutex);
            pthread_exit(NULL);
        }
        pthread_mutex_unlock(&mutex);

        char* pwd = strdup(info->password);
        int idx = getPrefixLength(pwd);
        size_t len = strlen(pwd);
        // for (int i = idx; i < strlen(pwd); i++) {
        //     pwd[i] = 'a';
        // }
        long int startidx = 0;
        long int count = 0;
        getSubrange(len - idx, threadcount, threadid, &startidx, &count);
        unsigned long remain = startidx;
        for (size_t i = idx; i < len; i++) {
            unsigned long divisor = 1;
            for (size_t j = 0; j < len - i - 1; j++) {
                divisor *= 26;
            }
            unsigned long numshift = remain / divisor;
            remain = remain % divisor;
            assert(numshift < 26);
            pwd[i] = 'a' + numshift;
        }
        v2_print_thread_start(threadid, info->user, startidx, pwd);
        int tag = 2;
        int hashcount = 0;
        for (long int i = 0; i < count; i++) {
            pthread_mutex_lock(&mutex);
            if (found == 1) {
                tag = 1;
                pthread_mutex_unlock(&mutex);
                break;
            }
            hashcount ++;
            pthread_mutex_unlock(&mutex);
            char* hashed = crypt_r(pwd, "xx", &cdata);
            if (strcmp(hashed, info->hash) == 0) {
                pthread_mutex_lock(&mutex);
                found = 1;
                passwordfound = strdup(pwd);
                tag = 0;
                pthread_mutex_unlock(&mutex);
                break;
            } 
            int incr = incrementString(pwd);
            assert(incr == 1);
        }
        v2_print_thread_result(threadid, hashcount, tag);
        pthread_mutex_lock(&mutex);
        totalcount += hashcount;
        finishthread += 1;
        if (finishthread == threadcount) {
            pthread_cond_broadcast(&cv);
        }
        while(finishthread < threadcount) {
            pthread_cond_wait(&cv, &mutex);
        }
        threadexit++;
        if (threadexit == threadcount) {
            pthread_cond_signal(&cv2);
        }
        pthread_mutex_unlock(&mutex);
        free(pwd);
    }
}

int start(size_t thread_count) {
    // TODO your code here, make sure to use thread_count!
    // Remember to ONLY crack passwords in other threads
    //create work thread;
    pthread_t threads[thread_count];
    pthread_cond_init(&cv, NULL);
    pthread_cond_init(&cv2, NULL);
    int * idx = malloc(sizeof(int)*thread_count);
    char* input = NULL;
    size_t inputsize = 0;
    threadcount = thread_count;
    eof = 0;
    push = 0;
    passwordfound = NULL;
    threadexit = 0;
    for (size_t i = 0; i < thread_count; i++) {
        idx[i] = i + 1;
        pthread_create(threads + i, NULL, worker, idx+i);
    }
    while (1) {
        double start_time = getTime();
        double start_cpu_time = getCPUTime();  
        int inputlen = getline(&input,&inputsize,stdin); 
        if (inputlen == -1) {
            //waiting thread to finish.
            // queue_push(q, end);
            pthread_mutex_lock(&mutex);
            push = thread_count;
            pthread_cond_broadcast(&cv);
            eof = 1;
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_lock(&mutex);
        info = malloc(sizeof(struct task));
        info->user = malloc(sizeof(char)*9);
        info->hash = malloc(sizeof(char)*14);
        info->password = malloc(sizeof(char)*9);
        sscanf(input, "%s %s %s", info->user, info->hash, info->password);
        finishthread = 0;
        push = thread_count;
        found = 0;
        totalcount = 0;
        v2_print_start_user(info->user);
        threadexit = 0;
        pthread_cond_broadcast(&cv);
        while(threadexit < thread_count) {
            pthread_cond_wait(&cv2, &mutex);
        }
        v2_print_summary(info->user, passwordfound, totalcount, getTime() - start_time, getCPUTime() - start_cpu_time, 1-found);
        free(info->hash);
        free(info->user);
        free(info->password);
        free(info);
        free(passwordfound);
        passwordfound = NULL;
        info = NULL;
        pthread_mutex_unlock(&mutex);
    }
    for (size_t i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
    free(input);
    free(idx);
    pthread_cond_destroy(&cv);
    pthread_cond_destroy(&cv2);
    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}
