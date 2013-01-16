#include "sharedmem.h"

User_server * user_server_data;
Room_server * room_server_data;
int * server_ids_data;

void sharedmem_init(int * shm_nums) {
	int user_server_desc, room_server_desc, server_ids_desc;
	if ((user_server_desc = shmget(shm_nums[0], sizeof(User_server) * MAX_SERVERS_NUMBER * MAX_USERS_NUMBER, 0666 | IPC_CREAT)) == FAIL ||
	(room_server_desc = shmget(shm_nums[1], sizeof(Room_server) * MAX_SERVERS_NUMBER * MAX_USERS_NUMBER, 0666 | IPC_CREAT)) == FAIL ||
	(server_ids_desc = shmget(shm_nums[2], sizeof(int) * MAX_SERVERS_NUMBER, 0666 | IPC_CREAT)) == FAIL) {
		perror("Couldn't connect to shared memory.");
		exit(EXIT_FAILURE);	
	}

	if ((user_server_data = shmat(user_server_desc, NULL, 0)) == (void *)FAIL || (room_server_data = shmat(room_server_desc, NULL, 0)) == (void *)FAIL ||
	(server_ids_data = shmat(server_ids_desc, NULL, 0)) == (void *)FAIL) { /* maybe some operations finished successfully */
		user_server_data == (void *) FAIL ? 0 : shmdt(user_server_data);
		room_server_data == (void *) FAIL ? 0 : shmdt(room_server_data);
		server_ids_data == (void *) FAIL ? 0 : shmdt(server_ids_data);
		perror("Couldn't import shared memory to process memory.");
		exit(EXIT_FAILURE);
	}

}

void sharedmem_end() {
	shmdt(user_server_data);
	shmdt(room_server_data);
	shmdt(server_ids_data);
}
