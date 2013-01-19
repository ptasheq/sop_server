#include "sharedmem.h"

User_server * user_server_data = (void *) FAIL;
Room_server * room_server_data = (void *) FAIL;
int * server_ids_data = (void *) FAIL;
int * sems = NULL;

short sharedmem_init(int * shm_nums) {
	int i, flag, user_server_desc, room_server_desc, server_ids_desc;
	unsigned short memunits = MAX_SERVERS_NUMBER * MAX_USERS_NUMBER;
	if (!(sems = malloc(3 * sizeof(int)))) {
		perror("Couldn't allocate semaphore array.");
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
		return FAIL;
	}
	semctl(sems[0], 0, SETVAL, 1);
	for (i = 1; i < SEM_NUM; ++i) {
		if ((sems[i] = semget((!i) ? SEM_SERVER_IDS : (i == 1) ? SEM_USER_SERVER : SEM_ROOM_SERVER, 1, flag)) != FAIL && (flag | IPC_CREAT)) {
			semctl(sems[0], 0, SETVAL, 1);	
		}
		else {
			perror("Couldn't create semaphore.");
			semctl(sems[0], 0, IPC_RMID);
			if (i == 2)
				semctl(sems[1], 0, IPC_RMID);
			free(sems);
			return FAIL;
		}
	}

	if ((user_server_desc = shmget(shm_nums[0], sizeof(User_server) * memunits, flag)) == FAIL ||
	(room_server_desc = shmget(shm_nums[1], sizeof(Room_server) * memunits, flag)) == FAIL ||
	(server_ids_desc = shmget(shm_nums[2], sizeof(int) * MAX_SERVERS_NUMBER, flag)) == FAIL) { 
		perror("Couldn't get access to shared memory.");
		sharedmem_end();
		return FAIL;
	}

	if ((user_server_data = shmat(user_server_desc, NULL, 0)) == (void *)FAIL || 
	(room_server_data = shmat(room_server_desc, NULL, 0)) == (void *)FAIL ||
	(server_ids_data = shmat(server_ids_desc, NULL, 0)) == (void *)FAIL) { /* maybe some operations finished successfully */
		perror("Couldn't import shared memory to process memory.");
		sharedmem_end();
		return FAIL;	
	}

	if (flag | IPC_CREAT) { /* we are the first */
		semdown(0);
		semdown(1);
		semdown(2);
		for (i = 0; i < MAX_SERVERS_NUMBER; ++i) {
			server_ids_data[i] = 0;
		}
		for (i = 0; i < memunits; ++i) {
			user_server_data[i].user_name[0] = '\0';
			room_server_data[i].room_name[0] = '\0';
		}
		semup(2);
		semup(1);
		semup(0);
	}
	if (register_server() == FAIL) {
		perror("The array of servers is already full.");
		return FAIL;
	}
	return 0;
}

void sharedmem_end() {
	short i = 0, last = 1;

	user_server_data == (void *) FAIL ? 0 : shmdt(user_server_data);
	room_server_data == (void *) FAIL ? 0 : shmdt(room_server_data);
	server_ids_data == (void *) FAIL ? 0 : shmdt(server_ids_data);

	semdown(0);
	for (i = 0; i < MAX_SERVERS_NUMBER; ++i) {
		if (server_ids_data[i] && server_ids_data[i] != queue_id) {
			last = 0;
		}
		else if (server_ids_data[i] == queue_id) { /* removing the server from the array */
			server_ids_data[i] = 0;	
		}
	}
	if (last) { /* we are the last server */
		semdown(1);
		semdown(2);
		if (!semctl(sems[0], 0, GETNCNT) && !semctl(sems[1], 0, GETNCNT) && !semctl(sems[2], 0, GETNCNT)) {
			/* no one else is waiting */
			for (i = 0; i < SEM_NUM; ++i) {
				semctl(sems[i], 0, IPC_RMID);
			}
		}
		else { /* someone appeared in the meantime */
			for (i = 0; i < SEM_NUM; ++i) {
				semup(i);
			}
		}
	}
	else {
		semup(0);
	}
	
	free(sems);
}

short register_server() {
	int i = 0;

	semdown(0);
	while (i < MAX_SERVERS_NUMBER && server_ids_data[i]) {
		++i;
	}
	if (i == MAX_SERVERS_NUMBER) { 
		semup(0);
		return FAIL;
	}
	server_ids_data[i] = queue_id;
	semup(0);
	return 0;
}

short search_for_username(char * str) {
	int i = 0;

	return 0;
}

void semdown(short semindex) {
	static struct sembuf op = {0, -1, 0};
	semop(sems[semindex], &op, 1);
}

void semup(short semindex) {
	static struct sembuf op = {0, 1, 0};
	semop(sems[semindex], &op, 1);
}
