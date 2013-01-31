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
#define ADD_FLAG 0010
#define DEL_FLAG 0100

#define add_user_in_shmem(str, id) do_in_shmem(0 | ADD_FLAG, id, str)
#define add_room_in_shmem(str, id) do_in_shmem(1 | ADD_FLAG, id, str)
#define del_user_in_shmem(str, id) do_in_shmem(0 | DEL_FLAG, id, str)
#define del_room_in_shmem(str, id) do_in_shmem(1 | DEL_FLAG, id, str)


extern User_server * user_server_data;
extern Room_server * room_server_data;
extern int * server_ids_data;
extern int * sems;

short sharedmem_init(int *);
short sharedmem_end();
short register_server();
short do_in_shmem(short flag, const int, const char *);
short send_message_to_servers(short flag, const int, const Msg_chat_message *);
short get_list_from_shmem(const int, Msg_request_response *);

#endif
