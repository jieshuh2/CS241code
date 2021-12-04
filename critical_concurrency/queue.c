/**
 * critical_concurrency
 * CS 241 - Spring 2021
 */
#include "queue.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * This queue is implemented with a linked list of queue_nodes.
 */
typedef struct queue_node {
    void *data;
    struct queue_node *next;
} queue_node;

struct queue {
    /* queue_node pointers to the head and tail of the queue */
    queue_node *head, *tail;

    /* The number of elements in the queue */
    ssize_t size;

    /**
     * The maximum number of elements the queue can hold.
     * max_size is non-positive if the queue does not have a max size.
     */
    ssize_t max_size;

    /* Mutex and Condition Variable for thread-safety */
    pthread_cond_t cv;
    pthread_mutex_t m;
};

queue *queue_create(ssize_t max_size) {
    /* Your code here */
    queue* q = malloc(sizeof(struct queue));
    if (q == NULL) {
        return NULL;
    }
    q->max_size = max_size;
    q->size = 0;
    pthread_mutex_init(&q->m, NULL);
    pthread_cond_init(&q->cv, NULL);
    return q;
}

void queue_destroy(queue *this) {
    /* Your code here */
    pthread_mutex_lock(&this->m);
    queue_node * ptr = this->head;
    while(ptr != NULL) {
        queue_node * next = ptr->next;
        free(ptr);
        ptr = next;
    }
    pthread_mutex_unlock(&this->m);
    pthread_mutex_destroy(&this->m);
    pthread_cond_destroy(&this->cv);
    free(this);

}

void queue_push(queue *this, void *data) {
    /* Your code here */
    pthread_mutex_lock(&this->m);
    while (this->max_size > 0 && this->size == this->max_size) {
        pthread_cond_wait(&this->cv, &this->m); /*unlock mutex, wait, relock mutex*/
    }
    this->size ++;
    queue_node* node = malloc(sizeof(queue_node));
    node->data = data;
    node->next = NULL;
    if (this->tail == NULL) {
        this->tail = node;
        this->head = node;
    } else {
        this->tail->next = node;
        this->tail = node;
    }
    pthread_cond_broadcast(&this->cv);
    pthread_mutex_unlock(&this->m);
}

void *queue_pull(queue *this) {
    /* Your code here */
    pthread_mutex_lock(&this->m);
    while (this->size == 0) {
        pthread_cond_wait(&this->cv, &this->m); /*unlock mutex, wait, relock mutex*/
    }
    this->size--;
    queue_node * ptr = this->head;
    if (this->head == this->tail) {
        this->head = NULL;
        this->tail = NULL;
    } else {
        this->head = ptr->next;
    }
    void* data = ptr->data;
    free(ptr);
    pthread_cond_broadcast(&this->cv);
    pthread_mutex_unlock(&this->m);
    return data;
}
