#include "microsleep.h"

void microsleep(unsigned long micros) {
    struct timespec ts;
    ts.tv_sec = micros / 1000000ul;
    ts.tv_nsec = (micros % 1000000ul) * 1000;
    nanosleep(&ts, NULL);
}