#include "func.h"

int main() {
    start_init();
    interface();
    int_handle(SIGINT);
    return 0;
}