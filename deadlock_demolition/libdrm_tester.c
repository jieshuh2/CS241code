/**
 * deadlock_demolition
 * CS 241 - Spring 2021
 */
#include "libdrm.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static drm_t *drm1;
static drm_t *drm2;
void* myfunc1(void* ptr) {
    pthread_t thread_id = pthread_self();
    drm_wait(drm1, &thread_id);
    drm_wait(drm2, &thread_id);
    // sleep(10);
    drm_post(drm1, &thread_id);
    drm_post(drm2, &thread_id);
    return NULL;
}
void* myfunc2(void* ptr) {
    pthread_t thread_id = pthread_self();
    drm_wait(drm2, &thread_id);
    drm_wait(drm1, &thread_id);
    // sleep(10);
    drm_post(drm1, &thread_id);
    drm_post(drm2, &thread_id);
    return NULL;
}
int main() {
    drm1 = drm_init();
    drm2 = drm_init();
    // TODO your tests here
    pthread_t tid1, tid2;
    pthread_create(&tid1, NULL, myfunc1, NULL);
    pthread_create(&tid2, NULL, myfunc2, NULL);
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    drm_destroy(drm1);
    drm_destroy(drm2);
    return 0;
}
