#ifndef IOLOOP_H
#define IOLOOP_H

#include "libs.h"

#define IO_BUF_SIZE 20 
#define wants_exit(str) !strcmp(str, "[exit]")

void io_loop();


#endif
