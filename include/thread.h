#ifndef THREAD_H
#define THREAD_H

#include <signal.h>

#define SIGEND SIGTERM

extern int pdesc[2];

void semdown(int);
void semup(int);

#endif
