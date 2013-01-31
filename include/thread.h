#ifndef THREAD_H
#define THREAD_H

#include <signal.h>

#define SIGEND SIGTERM
#define SIGRESP SIGUSR1

extern int queue_id;
extern int pdesc[2], pdesc3[2];

void semdown(int);
void semup(int);

#endif
