#ifndef THREAD_H
#define THREAD_H

#include <signal.h>

#define SIGEND SIGTERM

<<<<<<< HEAD
=======
typedef void (*sa_handler)(int);
void set_signal(int, sa_handler);

>>>>>>> 23f69180b9bcc8344862ba0d18c10c2cb4bbbbef
#endif
