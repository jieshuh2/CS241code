/**
 * deadlock_demolition
 * CS 241 - Spring 2021
 */
#include "graph.h"
#include "libdrm.h"
#include "set.h"
#include <pthread.h>
static graph* g = NULL;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
struct drm_t {
    pthread_mutex_t m;
};

int isCyclic(void* vertex, set * visit, void* prev) {
    if (set_contains(visit, vertex)) {
        return 1;
    }
    set_add(visit, vertex);
    vector* neighbors = graph_neighbors(g, vertex);
    for (size_t i = 0; i < vector_size(neighbors); i++) {
        if (vector_get(neighbors, i) != prev && isCyclic(vector_get(neighbors, i), visit, vertex) == 1) {
            return 1;
        }
    }
    vector* anti = graph_antineighbors(g, vertex);
    for (size_t i = 0; i < vector_size(anti); i++) {
        if (vector_get(anti, i) != prev && isCyclic(vector_get(anti, i), visit, vertex) == 1) {
            return 1;
        }
    }
    vector_destroy(neighbors);
    vector_destroy(anti);
    return 0;
}
drm_t *drm_init() {
    /* Your code here */
    pthread_mutex_lock(&mutex);
    if (g == NULL) {
        g = shallow_graph_create();
    }
    drm_t* drm = malloc(sizeof(drm_t));
    pthread_mutex_init(&drm->m,NULL);
    graph_add_vertex(g,drm);
    pthread_mutex_unlock(&mutex);
    return drm;
}

int drm_post(drm_t *drm, pthread_t *thread_id) {
    /* Your code here */
    pthread_mutex_lock(&mutex);
    if (!graph_contains_vertex(g, thread_id) || !graph_contains_vertex(g, drm)) {
        pthread_mutex_unlock(&mutex);
        return 0;
    } else {
        if (graph_adjacent(g, drm, thread_id)) {
            graph_remove_edge(g, drm, thread_id);
            pthread_mutex_unlock(&drm->m);
            pthread_mutex_unlock(&mutex);
            return 1;
        }
    }
    pthread_mutex_unlock(&mutex);
    return 0;
}

int drm_wait(drm_t *drm, pthread_t *thread_id) {
    /* Your code here */
    pthread_mutex_lock(&mutex);
    if (!graph_contains_vertex(g, thread_id)) {
        graph_add_vertex(g, thread_id);
    }
    if (graph_adjacent(g, drm, thread_id)) {
        pthread_mutex_unlock(&mutex);
        return 0;
    }
    // if (graph_adjacent(g, drm, thread_id) && graph_adjacent(g, thread_id, drm)) {
    //     graph_remove_edge(g, thread_id, drm);
    //     pthread_mutex_unlock(&mutex);
    //     return 0;
    // }
   
    graph_add_edge(g, drm, thread_id);
    set* visit = shallow_set_create();
    set_add(visit, drm);
    if (isCyclic(thread_id, visit, drm) == 1) {
        graph_remove_edge(g, drm, thread_id);
        set_destroy(visit);
        pthread_mutex_unlock(&mutex);
        return 0;
    } else {
        pthread_mutex_lock(&(drm->m));
        set_destroy(visit);
        pthread_mutex_unlock(&mutex);
        return 1;
    }
    set_destroy(visit);
    pthread_mutex_unlock(&mutex);
    return 0;
}

void drm_destroy(drm_t *drm) {
    /* Your code here */
    pthread_mutex_lock(&mutex);
    pthread_mutex_destroy(&drm->m);
    graph_remove_vertex(g, drm);
    free(drm);
    pthread_mutex_unlock(&mutex);
    return;
}
