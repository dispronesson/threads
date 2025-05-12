#include "func.h"

Queue* tqueue;
sem_t slots;
sem_t items;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER, print_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t tids_p[MAX_PRODUCER_COUNT];
pthread_t tids_c[MAX_CONSUMER_COUNT];
uint8_t producers;
uint8_t consumers;
uint8_t queue_capacity = START_QUEUE_CAPACITY;

void* producer(void* arg) {
    uint8_t id = (uint8_t)(uintptr_t)arg;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint32_t seedp = ts.tv_nsec ^ ts.tv_sec;

    while (1) {
        Message* msg = msg_create(&seedp);
        if (!msg) {
            pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
            err_handle("malloc", ENOMEM);
            abort();
        }
        pthread_cleanup_push(msg_destroy_void, msg);

        sem_wait(&slots);
        pthread_mutex_lock(&mutex);

        if (enqueue(tqueue, msg) == -1) {
            pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
            free(msg);
            err_handle("malloc", ENOMEM);
            abort();
        }
        uint16_t added = tqueue->added;

        pthread_mutex_unlock(&mutex);
        sem_post(&items);

        pthread_mutex_lock(&print_mutex);
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        printf("Producer %u added a new msg: ", id);
        printf("[type=%u,size=%u,data=0x", msg->type, msg->size == 0 ? 256 : msg->size);
        for(int i = 0; i < (msg->size == 0 ? 256 : msg->size); i++) printf("%02X", msg->data[i]);
        printf(",hash=0x%04X]. Count of added msgs: %u\n", msg->hash, added);
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_mutex_unlock(&print_mutex);

        pthread_cleanup_pop(0);

        sleep(3);
    }

    return NULL;
}

void* consumer(void* arg) {
    uint8_t id = (uint8_t)(uintptr_t)arg;

    while (1) {
        sem_wait(&items);
        pthread_mutex_lock(&mutex);

        Message* msg = dequeue(tqueue);
        u_int16_t extracted = tqueue->extracted;

        pthread_mutex_unlock(&mutex);
        sem_post(&slots);

        uint16_t hash = msg->hash;
        msg->hash = 0;
        msg->hash = compute_hash(msg);
        int passed = msg->hash == hash ? 1 : 0;

        pthread_mutex_lock(&print_mutex);
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        printf("Consumer %u extracted msg: ", id);
        printf("[type=%u,size=%u,data=0x", msg->type, msg->size == 0 ? 256 : msg->size);
        for(int i = 0; i < (msg->size == 0 ? 256 : msg->size); i++) printf("%02X", msg->data[i]);
        printf(",hash=0x%04X(%s)]. ", msg->hash, passed ? "PASSED" : "FAILED");
        printf("Count of extracted msgs: %u\n", extracted);
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_mutex_unlock(&print_mutex);

        msg_destroy(msg);

        sleep(3);
    }
}

Queue* queue_create() {
    Queue* queue = malloc(sizeof(Queue));
    if (!queue) {
        return NULL;
    }

    queue->head = queue->tail = NULL;
    queue->added = queue->extracted = 0;

    return queue;
}

int queue_destroy(Queue* queue) {
    if (!queue) {
        return -1;
    }

    Node* current_node = queue->head;
    while (current_node) {
        Node* temp_node = current_node;
        current_node = current_node->next;
        msg_destroy(temp_node->msg);
        free(temp_node);
    }

    free(queue);

    return 0;
}

int enqueue(Queue* queue, Message* msg) {
    if (!queue || !msg) {
        return -1;
    }

    Node* new_node = malloc(sizeof(Node));
    if (!new_node) {
        return -1;
    }

    new_node->msg = msg;
    new_node->next = NULL;

    if (queue->tail) {
        queue->tail->next = new_node;
    }
    else {
        queue->head = new_node;
    }

    queue->tail = new_node;
    queue->added++;

    return 0;
}

Message* dequeue(Queue* queue) {
    if (!queue || !queue->head) {
        return NULL;
    }
    
    Node* temp_node = queue->head;
    Message* msg = temp_node->msg;

    queue->head = temp_node->next;
    if (!queue->head) {
        queue->tail = NULL;
    }

    free(temp_node);
    queue->extracted++;

    return msg;
}

uint16_t compute_hash(Message* msg) {
    uint16_t hash = 0;

    hash ^= msg->type;
    hash ^= msg->hash;
    hash ^= msg->size;

    for (int i = 0; i < (msg->size == 0 ? 256 : msg->size); i++) {
        hash = (hash << 5) | (hash >> 11);
        hash ^= msg->data[i];
    }

    return hash;
}

Message* msg_create(uint32_t* seedp) {
    Message* msg = malloc(sizeof(Message));
    if (!msg) {
        return NULL;
    }

    uint16_t size;

    msg->hash = 0;
    msg->type = (uint8_t)(rand_r(seedp) % 256);

    while (1) {
        size = (uint16_t)(rand_r(seedp) % 257);

        if (size == 0) continue;

        if (size == 256) msg->size = 0;
        else msg->size = (uint8_t)size;

        break;
    }

    msg->data = malloc(((msg->size + 3) / 4) * 4);
    if (!msg->data) {
        free(msg);
        return NULL;
    }

    for (int i = 0; i < size; i++) {
        msg->data[i] = (uint8_t)rand_r(seedp);
    }

    msg->hash = compute_hash(msg);

    return msg;
}

int msg_destroy(Message* msg) {
    if (!msg) {
        return -1;
    }
    else {
        free(msg->data);
        free(msg);
        return 0;
    }
}

void msg_destroy_void(void* msg) {
    msg_destroy((Message*)msg);
}

void err_handle(char* msg, int en) {
    fprintf(stderr, "%s: %s\n", msg, strerror(en));
}

char getch() {
    struct termios oldt, newt;
    char ch;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    
    return ch;
}

void producer_create() {
    if (producers < MAX_PRODUCER_COUNT) {
        int res = pthread_create(&tids_p[producers], NULL, producer, (void*)(uintptr_t)(producers + 1));
        if (res == 0) {
            printf("Producer %u was created\n", ++producers);
        }
        else {
            err_handle("pthread_create", res);
        }
    }
    else {
        fprintf(stderr, "error: can't create more than 5 producers\n");
    }
}

void producer_delete() {
    if (producers > 0) {
        pthread_cancel(tids_p[producers - 1]);
        pthread_join(tids_p[producers - 1], NULL);
        printf("Producer %u was terminated\n", producers--);
    }
    else {
        fprintf(stderr, "error: there are no producers\n");
    }
}

void consumer_create() {
    if (consumers < MAX_CONSUMER_COUNT) {
        int res = pthread_create(&tids_c[consumers], NULL, consumer, (void*)(uintptr_t)(consumers + 1));
        if (res == 0) {
            printf("Consumer %u was created\n", ++consumers);
        }
        else {
            err_handle("pthread_create", res);
        }
    }
    else {
        fprintf(stderr, "error: can't create more than 5 consumers\n");
    }
}

void consumer_delete() {
    if (consumers > 0) {
        pthread_cancel(tids_c[consumers - 1]);
        pthread_join(tids_c[consumers - 1], NULL);
        printf("Consumer %u was terminated\n", consumers--);
    }
    else {
        fprintf(stderr, "error: there are no consumers\n");
    }
}

void inc_queue() {
    if (queue_capacity < MAX_QUEUE_CAPACITY) {
        sem_post(&slots);
        printf("Queue size increased. Total size: %u\n", ++queue_capacity);
    }
    else {
        fprintf(stderr, "error: queue size cannot be more than 20\n");
    }
}

void dec_queue() {
    if (queue_capacity > MIN_QUEUE_CAPACITY) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 3;

        if (sem_timedwait(&slots, &ts) == 0) {
            printf("Queue size decreased. Total size: %u\n", --queue_capacity);
        }
        else {
            fprintf(stderr, "error: cannot decrease queue size at this moment\n");
        }
    }
    else {
        fprintf(stderr, "error: queue size cannot be less than 1\n");
    }
}

void terminate_main_thread() {
    for (int i = 0; i < producers; i++) {
        pthread_cancel(tids_p[i]);
        pthread_join(tids_p[i], NULL);
    }

    for (int i = 0; i < consumers; i++) {
        pthread_cancel(tids_c[i]);
        pthread_join(tids_c[i], NULL);
    }
}

void interface() {
    char ch;

    while (1) {
        ch = getch();
        switch (ch) {
            case 'p': 
                producer_create();
                break;
            case 'c':
                consumer_create();
                break;
            case 'P':
                producer_delete();
                break;
            case 'C':
                consumer_delete();
                break;
            case '+':
                inc_queue();
                break;
            case '-':
                dec_queue();
                break;
            case 'q':
                terminate_main_thread();
                printf("Exiting...\n");
                return;
            default:
                fprintf(stderr, "error: incorrect key\n");
                break;
        }
    }
}