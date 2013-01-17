#include "sharedmem.h"

User_server * user_server_data;
Room_server * room_server_data;
int * server_ids_data;
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
		return FAIL;
	}
	semctl(sems[0], 0, SETVAL, 1);
	for (i = 1; i < 3; ++i) {
		if ((sems[i] = semget((!i) ? SEM_SERVER_IDS : (i == 1) ? SEM_USER_SERVER : SEM_ROOM_SERVER, 1, flag)) != FAIL) {
			semctl(sems[0], 0, SETVAL, 1);	
		}
		else {
			perror("Couln't create semaphore.");
			semctl(sems[0], 0, IPC_RMID);
			if (i == 2)
				semctl(sems[1], 0, IPC_RMID);
			return FAIL;
		}
	}

	if ((user_server_desc = shmget(shm_nums[0], sizeof(User_server) * memunits, flag)) == FAIL ||
	(room_server_desc = shmget(shm_nums[1], sizeof(Room_server) * memunits, flag)) == FAIL ||
	(server_ids_desc = shmget(shm_nums[2], sizeof(int) * MAX_SERVERS_NUMBER, flag)) == FAIL) { 
		perror("Couldn't get access to shared memory.");
		return FAIL;
	}

	if ((user_server_data = shmat(user_server_desc, NULL, 0)) == (void *)FAIL || 
	(room_server_data = shmat(room_server_desc, NULL, 0)) == (void *)FAIL ||
	(server_ids_data = shmat(server_ids_desc, NULL, 0)) == (void *)FAIL) { /* maybe some operations finished successfully */
		user_server_data == (void *) FAIL ? 0 : shmdt(user_server_data);
		room_server_data == (void *) FAIL ? 0 : shmdt(room_server_data);
		server_ids_data == (void *) FAIL ? 0 : shmdt(server_ids_data);
		perror("Couldn't import shared memory to process memory.");
		return FAIL;	
	}
	return 0;
}

void sharedmem_end() {
	free(sems);
	shmdt(user_server_data);
	shmdt(room_server_data);
	shmdt(server_ids_data);
}
