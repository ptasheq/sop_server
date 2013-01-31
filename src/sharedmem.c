#include "sharedmem.h"
#include "thread.h"
#include "init.h"

User_server * user_server_data = (void *) FAIL;
Room_server * room_server_data = (void *) FAIL;
int * server_ids_data = (void *) FAIL;
int * sems = NULL, * shmids = NULL;

short sharedmem_init(int * shm_nums) {
	int i, flag;
	unsigned short memunits = MAX_SERVERS_NUMBER * MAX_USERS_NUMBER;
	if (!(sems = malloc(3 * sizeof(int))) || !(shmids = malloc(3 * sizeof(int)))) {
		perror("Couldn't allocate required arrays.");
		return FAIL;
	}
	
	if ((sems[0] = semget(SEM_SERVER_IDS, 1, 0666 | IPC_CREAT | IPC_EXCL)) != FAIL) {
		flag = 0666 | IPC_CREAT | IPC_EXCL;
	}
	else if ((sems[0] = semget(SEM_SERVER_IDS, 1, 0666)) != FAIL) {
		flag = 0666;
	}
	else {
		perror("Couldn't create semaphore.");
		free(sems);
		free(shmids);
		return FAIL;
	}
	semctl(sems[0], 0, SETVAL, 1);
	for (i = 1; i < SEM_NUM; ++i) {
		if ((sems[i] = semget((!i) ? SEM_SERVER_IDS : (i == 1) ? SEM_USER_SERVER : SEM_ROOM_SERVER, 1, flag)) != FAIL) {
			if (flag & IPC_CREAT) {
				semctl(sems[i], 0, SETVAL, 1);	
			}
		}
		else {
			perror("Couldn't create semaphore.");
			semctl(sems[0], 0, IPC_RMID);
			if (i == 2)
				semctl(sems[1], 0, IPC_RMID);
			free(sems);
			free(shmids);
			return FAIL;
		}
	}
	
	for (i = 0; i < SHM_NUM; ++i) {
		if ((shmids[i] = shmget(shm_nums[i], (!i) ? sizeof(int) * MAX_SERVERS_NUMBER : 
		(i == 1) ? sizeof(User_server) * memunits : sizeof(Room_server) * memunits, flag)) == FAIL) {
			perror("Couldn't get access to shared memory.");
			sharedmem_end();
			return FAIL;
		}
	}
	if ((server_ids_data = shmat(shmids[0], NULL, 0)) == (void *)FAIL || 
	(user_server_data = shmat(shmids[1], NULL, 0)) == (void *)FAIL ||
	(room_server_data = shmat(shmids[2], NULL, 0)) == (void *)FAIL) { /* maybe some operations finished successfully */
		perror("Couldn't import shared memory to process memory.");
		sharedmem_end();
		return FAIL;	
	}

	if (flag & IPC_CREAT)  { /* we are the first */
		semdown(sems[0]);
		semdown(sems[1]);
		semdown(sems[2]);
		for (i = 0; i < MAX_SERVERS_NUMBER; ++i) {
			server_ids_data[i] = -1;
		}
		for (i = 0; i < memunits; ++i) {
			user_server_data[i].server_id = -1;
			room_server_data[i].server_id = -1;
		}
		semup(sems[2]);
		semup(sems[1]);
		semup(sems[0]);
	}
	if (register_server() == FAIL) {
		perror("The array of servers is already full.");
		return FAIL;
	}
	return 0;
}

short sharedmem_end() {
	short i = 0, last = 1;
	semdown(sems[0]);
	for (i = 0; i < MAX_SERVERS_NUMBER; ++i) {
		if (server_ids_data[i] > -1 && server_ids_data[i] != queue_id) {
			last = 0;
		}
		else if (server_ids_data[i] == queue_id) { /* removing the server from the array */
			server_ids_data[i] = -1;	
		}
	}
	if (last) { /* we are the last server */
		semdown(sems[1]);
		semdown(sems[2]);
		if (!semctl(sems[0], 0, GETNCNT) && !semctl(sems[1], 0, GETNCNT) && !semctl(sems[2], 0, GETNCNT)) {
			/* no one else is waiting */
			for (i = 0; i < SEM_NUM; ++i) {
				semctl(sems[i], 0, IPC_RMID);
				shmctl(shmids[i], IPC_RMID, NULL);
			}
		}
		else { /* someone appeared in the meantime */
			for (i = 0; i < SEM_NUM; ++i) {
				semup(sems[i]);
			}
		}
	}
	else {
		semup(sems[0]);
	}
	user_server_data == (void *) FAIL ? 0 : shmdt(user_server_data);
	room_server_data == (void *) FAIL ? 0 : shmdt(room_server_data);
	server_ids_data == (void *) FAIL ? 0 : shmdt(server_ids_data);
	free(shmids);	
	free(sems);
	return last;
}

short register_server() {
	int i = 0;
	semdown(sems[0]);
	while (i < MAX_SERVERS_NUMBER && server_ids_data[i] > -1 ) {
		++i;
	}
	if (i == MAX_SERVERS_NUMBER) { 
		semup(sems[0]);
		return FAIL;
	}
	server_ids_data[i] = queue_id;
	semup(sems[0]);
	return 0;
}

short do_in_shmem(short flag, const int server_id, const char * str) {
	short i = 0, empty = -1, given = -1, n = 0, size = MAX_SERVERS_NUMBER * MAX_USERS_NUMBER;
	if (!(flag & 1)) { /* user_server */
		semdown(sems[1]);
		while (i < size) {
			if (user_server_data[i].server_id ==-1)
				++n;
			if (empty < 0 && user_server_data[i].server_id == -1)
				empty = i;
			if (user_server_data[i].server_id > -1 && !strcmp(str, user_server_data[i].user_name))
				given = i;
			++i;
		}
		if ((flag & ADD_FLAG) && empty > -1 && given == -1) { /* we have room and string is unique */
			strcpy(user_server_data[empty].user_name, str);
			user_server_data[empty].server_id = server_id;
		}
		else if ((flag & DEL_FLAG) && given > -1) {
			user_server_data[given].server_id = -1;
		}
		semup(sems[1]);
	} 
	else { /* room_server */
		semdown(sems[2]);
		while (i < size) {
			if (empty < 0 && room_server_data[i].server_id == -1)
				empty = i;
			if (room_server_data[i].server_id == server_id && !strcmp(str, room_server_data[i].room_name))
				given = i;
			++i;
		}

		if ((flag & ADD_FLAG) && empty > -1 && given == -1) { /* we have room and string is unique */
			strcpy(room_server_data[empty].room_name, str);
			room_server_data[empty].server_id = server_id;
		}
		else if ((flag & DEL_FLAG) && given > -1) {
			room_server_data[given].server_id = -1;
		}
		semup(sems[2]);
	}
	return ((flag & ADD_FLAG) && given == -1 && empty > -1) || ((flag & DEL_FLAG) && given > -1) ? 0 : FAIL;
}

short send_message_to_servers(short flag, const int sender_id, const Msg_chat_message * msg) {
	short i = 0, j = 0, sent_count = 0, size = MAX_SERVERS_NUMBER * MAX_USERS_NUMBER;	
	s2s_data->server_ipc_num = queue_id;
	if (flag == PRIVATE) { /* we want to send message to other user */
		semdown(sems[1]);
		while (i < size && strcmp(user_server_data[i].user_name, msg->receiver) || user_server_data[i].server_id == sender_id)
			++i;
		if (i < size) {
			if (send_message(user_server_data[i].server_id, s2s_data->type, s2s_data) != FAIL) {
				while (j < MAX_FAILS && receive_message(queue_id, SERVER2SERVER, s2s_data) == FAIL && 
				s2s_data->server_ipc_num != user_server_data[i].server_id) {
					++j;
					msleep(5);
			}
				if (j < MAX_FAILS && send_message(user_server_data[i].server_id, msg->type, msg) != FAIL)
				++sent_count;
			}
		}
		semup(sems[1]);
	}
	else { /* we want to public message */
		semdown(sems[2]);
		for (; i < size; ++i) {
			if (room_server_data[i].server_id != sender_id && !strcmp(room_server_data[i].room_name, msg->receiver)) {
				if (send_message(room_server_data[i].server_id, s2s_data->type, s2s_data) != FAIL)
					while (j < MAX_FAILS && receive_message(queue_id, SERVER2SERVER, s2s_data) == FAIL && 
					s2s_data->server_ipc_num != user_server_data[i].server_id) {
						if (j < MAX_FAILS && send_message(room_server_data[i].server_id, msg->type, msg) != FAIL)
							++sent_count;
					}
				}
			s2s_data->server_ipc_num = queue_id;
		}
		semup(sems[2]);
	}
	return (sent_count) ? 0 : FAIL;
}

short get_list_from_shmem(const int type, Msg_request_response * ptr) {
	short i, j, size = MAX_SERVERS_NUMBER*MAX_USERS_NUMBER;
	if (type == USERS_LIST) {
		semdown(sems[1]);
		for (i = 0; i < size; ++i) {
			if (user_server_data[i].server_id > -1)
				strcpy(ptr->names[i], user_server_data[i].user_name);
			else
				ptr->names[i][0] = '\0';
		}
		semup(sems[1]);
		return 0;
	}
	else if (type == ROOMS_LIST)  {
		semdown(sems[2]);
		for (i = 0; i < size; ++i) {
			if (room_server_data[i].server_id > -1)
				strcpy(ptr->names[i], room_server_data[i].room_name);
			else
				ptr->names[i][0] = '\0';
		}
		semup(sems[2]);
		for (i = 0; i < size; ++i) {
			for (j = i+1; j < size; ++j) {
				if (!strcmp(ptr->names[i], ptr->names[j])) /* if name is unique */
					ptr->names[i][0] = '\0';
			}
		}
		return 0;
	}
	return FAIL;
}

void semdown(int sem) {
	static struct sembuf op = {0, -1, 0};
	semop(sem, &op, 1);
}

void semup(int sem) {
	static struct sembuf op = {0, 1, 0};
	semop(sem, &op, 1);
}
