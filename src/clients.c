#include "clients.h"
#include "protocol.h"
#include "init.h"
#include "sharedmem.h"
#include "structmem.h"
#include "thread.h"
#include <time.h>

Msg_login * login_data = NULL;
Msg_response * response_data = NULL;
Msg_room * room_data = NULL;
Msg_chat_message * chatmsg_data = NULL;
int * clients_queues = NULL;
int queue_id;
int Pdesc[2];

void client_service_init(int * ipcs) {
	if (pipe(Pdesc) == FAIL) {
		perror("Couldn't create pipe.");
		exit(EXIT_FAILURE);
	}

	if (fcntl(Pdesc[0], F_SETFL, 0666 | O_NONBLOCK) == FAIL) {
		perror("Couldn't set pipe flag.");
		exit(EXIT_FAILURE);
	}
	if (!(clientsrv_pid = fork())) {
		if ((queue_id = msgget(ipcs[0], 0666 | IPC_CREAT)) == FAIL) {
			perror("Couldn't create message queue.");
			exit(EXIT_FAILURE);
		}
		if (sharedmem_init(&ipcs[1]) == FAIL) {
			msgctl(queue_id, IPC_RMID, NULL);
			exit(EXIT_FAILURE);
		}
		if (!allocate_mem(LOGIN, &login_data) || !allocate_mem(RESPONSE, &response_data) || 
			!allocate_mem(ROOM, &room_data) || !allocate_mem(MESSAGE, &chatmsg_data)) {
			perror("Couldn't allocate data structures.");
			client_service_end(0);
		}
		
		/*if (!(clients = malloc(MAX_USERS_NUMBER * sizeof(int))*/

		client_service();
	}
	else if	(clientsrv_pid == FAIL) {
		perror("Couldn't create client thread\n");
		exit(EXIT_FAILURE);
	}
}

void client_service() {
	set_signal(SIGEND, client_service_end);
	int client_id;
	int err_flag, received, i = 0, cur_client;
	time_t t = time(NULL);
	while (1) {
		if (receive_message(queue_id, LOGIN, login_data) != FAIL) {
			perform_action(LOGIN);
			sprintf(response_data->content, "%s%s", "Welcome, ", login_data->username);
			if (send_message(client_id, response_data->type, response_data)) {
				++i; 
			}
		}
		else if (receive_message(queue_id, MESSAGE, chatmsg_data) != FAIL) {
			response_data->response_type = MSG_SEND;	
			send_message(client_id, response_data->type, response_data);
			perror(chatmsg_data->message);
		}
		else if (receive_message(queue_id, ROOM, room_data) != FAIL) {
			perror(room_data->room_name);
			response_data->response_type = ENTERED_ROOM_SUCCESS;
			send_message(client_id, response_data->type, response_data);
		}
		if (time(NULL) - t >= 1) {
			response_data->response_type = PING;
			send_message(client_id, response_data->type, response_data);
			t = time(NULL);
		}
	}
}

void client_service_end(Flag flag) { /* != 0 - raised by signal, ==0 - error */
	free_mem(login_data);
	free_mem(response_data);
	free_mem(chatmsg_data);
	free_mem(room_data);

	sharedmem_end();
	if (flag)
		exit(EXIT_SUCCESS);
	else
		exit(EXIT_FAILURE);
}

void perform_action(unsigned const int msgtype) {
	if (msgtype == LOGIN) {
	}
}
