#ifndef STRUCTMEM_H
#define STRUCTMEM_H

#include "libs.h"
#include "protocol.h"

#define allocate_mem(type, ptr) am(type, (void **) ptr)
#define free_mem(ptr) ptr == NULL ? 0 : free(ptr); ptr = NULL;

void * am(const int, void **);

#endif
