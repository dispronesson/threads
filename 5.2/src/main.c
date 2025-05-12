#include "func.h"

int main() {
    tqueue = queue_create();
    if (!tqueue) {
        err_handle("malloc", ENOMEM);
        abort();
    }

    interface();

    queue_destroy(tqueue);

    return 0;
}