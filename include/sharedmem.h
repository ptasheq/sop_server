#ifndef	SHAREDMEM_H
#define SHAREDMEM_H

#include "libs.h"
#include <sys/shm.h>
#include "protocol.h"

extern User_server * user_server_data;
extern Room_server * room_server_data;
extern int * server_ids_data;

void sharedmem_init(int *);
void sharedmem_end();

#endif
