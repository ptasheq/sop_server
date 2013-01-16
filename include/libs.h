#ifndef LIBS_H

#define LIBS_H

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define STR_BUF_SIZE 256
#define MAX_FAILS 5
#define FAIL -1
#define ARG_NUM 4 

typedef const unsigned int Flag;

#endif
