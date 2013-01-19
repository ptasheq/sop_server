#ifndef	SHAREDMEM_H
#define SHAREDMEM_H

#include "libs.h"
#include <sys/shm.h>
#include <sys/sem.h>
#include "protocol.h"

#define SEM_SERVER_IDS 35
#define SEM_USER_SERVER 36
#define SEM_ROOM_SERVER 37
#define SEM_NUM 3
#define SHM_NUM SEM_NUM

extern User_server * user_server_data;
extern Room_server * room_server_data;
extern int * server_ids_data;
extern int * sems;

short sharedmem_init(int *);
void sharedmem_end();
short register_server();
void semdown(short);
void semup(short);

#endif
