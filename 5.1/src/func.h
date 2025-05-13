#ifndef FUNC_H
#define FUNC_H

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>

#define START_QUEUE_CAPACITY 10
#define MAX_QUEUE_CAPACITY 20
#define MIN_QUEUE_CAPACITY 1
#define MAX_PRODUCER_COUNT 5
#define MAX_CONSUMER_COUNT 5

typedef struct Message {
    uint8_t type;
    uint16_t hash;
    uint8_t size;
    uint8_t* data;
} Message;

typedef struct Node {
    Message* msg;
    struct Node* next;
} Node;

typedef struct Queue {
    Node* head;
    Node* tail;
    uint16_t added;
    uint16_t extracted;
} Queue;

extern Queue* tqueue;
extern sem_t slots;
extern sem_t items;
extern uint8_t queue_capacity;

void* producer(void* arg);
void* consumer(void * arg);
Queue* queue_create();
int queue_destroy(Queue* queue);
int enqueue(Queue* queue, Message* msg);
Message* dequeue(Queue* queue);
uint16_t compute_hash(Message* msg);
Message* msg_create(uint32_t* seedp);
int msg_destroy(Message* msg);
void msg_destroy_void(void* msg);
void err_handle(char* msg, int en);
void start_init();
void producer_create();
void producer_delete();
void consumer_create();
void consumer_delete();
void inc_queue();
void dec_queue();
void terminate_main_thread();
void interface();
void int_handle(int signo);

#endif //FUNC_H