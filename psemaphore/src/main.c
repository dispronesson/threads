#include "func.h"

int main() {
    tqueue = queue_create();
    if (!tqueue) {
        err_handle("malloc", ENOMEM);
        abort();
    }

    sem_init(&slots, 0, START_QUEUE_CAPACITY);
    sem_init(&items, 0, 0);

    interface();

    sem_destroy(&items);
    sem_destroy(&slots);
    queue_destroy(tqueue);

    return 0;
}