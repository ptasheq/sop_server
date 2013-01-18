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
		sharedmem_end();
		perror("Couldn't get access to shared memory.");
		return FAIL;
	}

	if ((user_server_data = shmat(user_server_desc, NULL, 0)) == (void *)FAIL || 
	(room_server_data = shmat(room_server_desc, NULL, 0)) == (void *)FAIL ||
	(server_ids_data = shmat(server_ids_desc, NULL, 0)) == (void *)FAIL) { /* maybe some operations finished successfully */
		sharedmem_end();
		perror("Couldn't import shared memory to process memory.");
		return FAIL;	
	}

	if (flag | IPC_CREAT) { /* we are the first */
		struct sembuf op;
		op.sem_op = -1;
		op.sem_num = 0;
		semop(sems[0], &op, 1);
		semop(sems[1], &op, 1);
		semop(sems[2], &op, 1);
		for (i = 0; i < MAX_SERVERS_NUMBER; ++i) {
			server_ids_data[i] = 0;
		}
		for (i = 0; i < memunits; ++i) {
			user_server_data[i].user_name[0] = '\0';
			room_server_data[i].room_name[0] = '\0';
		}
		op.sem_op = 1;
		semop(sems[2], &op, 1);
		semop(sems[1], &op, 1);
		semop(sems[0], &op, 1);
	}
	return 0;
}

void sharedmem_end() {
	struct sembuf op;
	short i = 0;

	user_server_data == (void *) FAIL ? 0 : shmdt(user_server_data);
	room_server_data == (void *) FAIL ? 0 : shmdt(room_server_data);
	server_ids_data == (void *) FAIL ? 0 : shmdt(server_ids_data);

	op.sem_num = 0;
	op.sem_op = -1;
	semop(sems[0], &op, 1);
	for (i = 0; i < MAX_SERVERS_NUMBER; ++i) {
		if (server_ids_data[i] && server_ids_data[i] != queue_id) {
			break;
		}
	}
	if (i == MAX_SERVERS_NUMBER) { /* we are the last server */
		semop(sems[1], &op, 1);
		semop(sems[2], &op, 1);
		op.sem_op = 1;
		if (!semctl(sems[0], 0, GETNCNT) && !semctl(sems[1], 0, GETNCNT) && !semctl(sems[2], 0, GETNCNT)) {
			/* no one else is waiting */
			for (i = 0; i < SEM_NUM; ++i) {
				semctl(sems[i], 0, IPC_RMID);
			}
		}
		else { /* someone appeared in the meantime */
			for (i = 0; i < SEM_NUM; ++i) {
				semop(sems[i], &op, 1);
			}
		}
	}
	
	free(sems);
}
