#ifndef	SHAREDMEM_H
#define SHAREDMEM_H

#include "libs.h"
#include <sys/shm.h>
#ifndef PROTOCOL_H

#define MAX_SERVERS_NUMBER 15
#define MAX_USERS_NUMBER 20
#define USER_NAME_MAX_LENGTH 10
#define ROOM_NAME_MAX_LENGTH 10

#endif

typedef struct {
	char user_name[USER_NAME_MAX_LENGTH];
	int server_id;
} User_server;

typedef struct {
	char room_name[ROOM_NAME_MAX_LENGTH];
	int server_id;
} Room_server;

extern User_server * user_server_data;
extern Room_server * room_server_data;
extern int * server_ids_data;

void sharedmem_init(int *);
void sharedmem_end();

#endif
