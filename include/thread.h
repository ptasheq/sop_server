#ifndef THREAD_H
#define THREAD_H

#include <signal.h>

#define SIGEND SIGTERM

typedef void (*sa_handler)(int);
void set_signal(int, sa_handler);

#endif
