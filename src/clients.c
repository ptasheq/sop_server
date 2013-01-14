#include "clients.h"
#include "protocol.h"
#include "thread.h"
#include <time.h>

Msg_login login_data = {LOGIN, "", 0};
Msg_response response_data = {RESPONSE, 0, ""};
Msg_room room_data = {ROOM, 0, "", ""};
Msg_chat_message chatmsg_data = {MESSAGE, 0, "", "", "", ""};
int connected_clients;
int Pdesc[2];

void client_service_init() {
	if (pipe(Pdesc) == FAIL) {
		perror("Couldn't create pipe.");
		exit(EXIT_FAILURE);
	}

	if (fcntl(Pdesc[0], F_SETFL, 0666 | O_NONBLOCK) == FAIL) {
		perror("Couldn't set pipe flag.");
		exit(EXIT_FAILURE);
	}
	client_service_init();
}

void client_service() {
	int fv;
	if ((fv = fork())) {
		set_signal(SIGEND, client_service_end);
		int client_id;
	    int err_flag, received, i = 0, cur_client;
		time_t t = time(NULL);
    	while (1) {
			for (cur_client = 0; cur_client < connected_clients; ++cur_client) {
				if (receive_message(ipc_id, LOGIN, &login_data) != FAIL) {
					sprintf(response_data.content, "%s%s", "Welcome, ", login_data.username);
					if (send_message(client_id, response_data.type, &response_data)) {
						++i; 
					}
				}
				else if (receive_message(ipc_id, MESSAGE, &chatmsg_data) != FAIL) {
					response_data.response_type = MSG_SEND;	
					send_message(client_id, response_data.type, &response_data);
					perror(chatmsg_data.message);
				}
				else if (receive_message(ipc_id, ROOM, &room_data) != FAIL) {
					perror(room_data.room_name);
					response_data.response_type = ENTERED_ROOM_SUCCESS;
					send_message(client_id, response_data.type, &response_data);
				}
				if (i && time(NULL) - t >= 1) {
					response_data.response_type = PING;
					send_message(client_id, response_data.type, &response_data);
					t = time(NULL);
				}
			}
			
		}
	}
	else if	(fv == FAIL) {
		perror("Couldn't create client thread\n");
		exit(EXIT_FAILURE);
	}
}

void client_service_end() {
	exit(EXIT_SUCCESS);
}

int add_user(char * usrname) {
	if (1) {
		return 1;
	}
	else {
		return FAIL;
	}
}

void set_signal(int signum, sa_handler handler) {
	struct sigaction sh;
	sh.sa_handler = handler;
	sigemptyset(&sh.sa_mask);
	sh.sa_flags = 0;
	sigaction(signum, &sh, NULL);
}
