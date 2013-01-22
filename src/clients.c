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
Msg_request * request_data = NULL;
Msg_request_response * request_response_data = NULL;
Room_user * room_user_data = NULL;
int clients = 0;
int queue_id;
int pdesc[2], pdesc2[2];

void client_service_init(int * ipcs) {
	if (pipe(pdesc) == FAIL || pipe(pdesc2) == FAIL) {
		perror("Couldn't create pipe");
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
			!allocate_mem(ROOM, &room_data) || !allocate_mem(MESSAGE, &chatmsg_data) ||
			!allocate_mem(RESPONSE, &request_data) || !allocate_mem(USERS, &request_response_data) || 
			!allocate_mem(ROOM_USER, &room_user_data)) {
			perror("Couldn't allocate data structures.");
			client_service_end(0);
		}
		short i = 0;
		while (i < MAX_USERS_NUMBER) {
			room_user_data[i].roomname[0] = '\0';
			++i;
		}
		inform_log_service(SERVER_REGISTERED, "", "");
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
	/*	if (time(NULL) - t >= 1) {
			response_data->response_type = PING;
			send_message(client_id, response_data->type, response_data);
			t = time(NULL);
		}*/
		msleep(10);
	}
}

void client_service_end(Flag flag) { /* != 0 - raised by signal, ==0 - error */
	set_signal(SIGPIPE, SIG_IGN);
	short last = sharedmem_end();
	write(pdesc[1], &last, sizeof(short));
	free_mem(login_data);
	free_mem(response_data);
	free_mem(chatmsg_data);
	free_mem(room_data);
	close(pdesc[1]);
	close(pdesc2[0]);
	if (flag)
		exit(EXIT_SUCCESS);
	else
		exit(EXIT_FAILURE);
}

void perform_action(unsigned const int msgtype) {
	int i = 0;
	if (msgtype == LOGIN) {
		response_data->response_type = (clients < MAX_USERS_NUMBER && add_user_in_shmem(login_data->username, queue_id) != FAIL) ? 
		LOGIN_SUCCESS : LOGIN_FAILED;
		if (response_data->response_type == LOGIN_SUCCESS)
			++clients;

	}
	else if (msgtype == LOGOUT) {
		response_data->response_type = (clients > 0 && del_user_in_shmem(login_data->username, queue_id) != FAIL) ?
		LOGOUT_SUCCESS : LOGOUT_FAILED;
		if (response_data->response_type == LOGOUT_SUCCESS)
			--clients;
	}
	else if (msgtype == REQUEST) {
		request_response_data->type = (request_data->request_type == USERS_LIST) ? USERS : 
		(request_data->request_type == ROOMS_LIST) ? ROOMS : ROOM_USERS;
		if (request_data->request_type == ROOM_USERS_LIST) {

		}
		else {
			get_list_from_shmem(request_data->request_type, request_response_data);	
		}
	}
	else if (msgtype == ROOM) {
		int i = 0, empty = -1, more_in_room = 0;
		if (room_data->operation_type == ENTER_ROOM || room_data->operation_type == CHANGE_ROOM) {
			while (i < MAX_USERS_NUMBER) {
				if (room_data->operation_type == CHANGE_ROOM &&	!strcmp(room_user_data[i].username, room_data->user_name)) {
					/* wants change room and found the same username */
					if (!strcmp(room_user_data[i].roomname, room_data->room_name)) { /* already in the same room */
						empty = -1;
						i = MAX_USERS_NUMBER;
					}
					else {
						empty = i;
					}
					break;
				}
				else if (!room_user_data[i].roomname[i] && empty < 0) { /* found empty place */
					empty = i;
				}
				++i;		
			}
			if (room_data->operation_type == CHANGE_ROOM) {
				if (i < MAX_USERS_NUMBER && empty > -1) {
					for (i = 0; i < MAX_USERS_NUMBER; ++i) {
						if (room_user_data[i].roomname[0] && !strcmp(room_user_data[i].roomname, room_data->room_name) && i != empty)
							break;
					}
					if (i == MAX_USERS_NUMBER) { /* Previous room should be deleted */
						del_room_in_shmem(room_user_data[empty].roomname, queue_id);
					}
				}
			}
			if (empty > -1 && add_room_in_shmem(room_data->room_name, queue_id) != FAIL) {
				strcpy(room_user_data[empty].roomname, room_data->room_name);
				strcpy(room_user_data[empty].username, room_data->user_name);
				response_data->response_type = (room_data->operation_type == CHANGE_ROOM) ? CHANGE_ROOM_SUCCESS : ENTERED_ROOM_SUCCESS;
			}
			else {
				response_data->response_type = (room_data->operation_type == CHANGE_ROOM) ? CHANGE_ROOM_FAILED : ENTERED_ROOM_FAILED;
			}
		}
		if (room_data->operation_type == LEAVE_ROOM) {
			while (i < MAX_USERS_NUMBER) {
				if (!strcmp(room_user_data[i].roomname, room_data->room_name) && room_data->room_name[0]) {
					if (!strcmp(room_user_data[i].username, room_data->user_name))
						empty = i;
					else
						more_in_room = 1;
				}
				++i;	
			}
			if (empty > -1) {
				if (!more_in_room)
					del_room_in_shmem(room_user_data[empty].roomname, queue_id);
				room_user_data[empty].roomname[0] = '\0';
				response_data->response_type = LEAVE_ROOM_SUCCESS;
			}
			else
				response_data->response_type = LEAVE_ROOM_FAILED;
		}
	}
	else if (msgtype == MESSAGE) {
		while (i < MAX_USERS_NUMBER && !strcmp(chatmsg_data->sender, room_user_data[i].username)) 
			++i;
		if (i < MAX_USERS_NUMBER) {
			int j = 0;
			if (chatmsg_data->msg_type == PRIVATE) {
				while (j < MAX_USERS_NUMBER && strcmp(chatmsg_data->receiver, room_user_data[j].username))
					++j;
				if (j == MAX_USERS_NUMBER) { /* user not found, so checking if there is such room */
					j = 0;
					while (j < MAX_USERS_NUMBER && strcmp(chatmsg_data->receiver, room_user_data[j].roomname))
						++j;
				}
			}
		}
	}
	write(pdesc[1], &msgtype, sizeof(int));
}

void inform_log_service(unsigned short type, const char * user, const char * room) {
	char control_byte;
	write(pdesc[1], &type, sizeof(short));
	read(pdesc2[0], &control_byte, 1);
	write(pdesc[1], "cos", 4);
	read(pdesc2[0], &control_byte, 1);
	write(pdesc[1], "cos2", 5);
	read(pdesc2[0], &control_byte, 1);
	perror("wysylamy");
}
