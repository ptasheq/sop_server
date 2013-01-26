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
		printf("Queue id: %d\n", queue_id);
		if (sharedmem_init(&ipcs[1]) == FAIL) {
			msgctl(queue_id, IPC_RMID, NULL);
			exit(EXIT_FAILURE);
		}
		if (!allocate_mem(LOGIN, &login_data) || !allocate_mem(RESPONSE, &response_data) || 
			!allocate_mem(ROOM, &room_data) || !allocate_mem(MESSAGE, &chatmsg_data) ||
			!allocate_mem(REQUEST, &request_data) || !allocate_mem(USERS, &request_response_data) || 
			!allocate_mem(ROOM_USER, &room_user_data)) {
			perror("Couldn't allocate data structures.");
			client_service_end(0);
		}
		short i = 0;
		while (i < MAX_USERS_NUMBER) {
			room_user_data[i].roomname[0] = '\0';
			room_user_data[i].username[0] = '\0';
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
	int err_flag, received, i = 0, cur_client;
	time_t t = time(NULL);
	response_data->type = RESPONSE;
	while (1) {
		if (receive_message(queue_id, LOGIN, login_data) != FAIL) {
			perform_action(LOGIN);
		}
		else if (receive_message(queue_id, LOGOUT, login_data) != FAIL) {
			perform_action(LOGOUT);
		}
		else if (receive_message(queue_id, MESSAGE, chatmsg_data) != FAIL) {
			perform_action(MESSAGE);
		}
		else if (receive_message(queue_id, ROOM, room_data) != FAIL) {
			perror(room_data->room_name);
			perform_action(ROOM);
		}
		if (time(NULL) - t >= PING_TIME && clients) {
			response_data->response_type = PING;
			for (i = 0; i < MAX_USERS_NUMBER; ++i) {
				if (room_user_data[i].username[0])
					send_message(room_user_data[i].id, response_data->type, response_data);
			}
			t = time(NULL);
		}
		msleep(5);
	}
}

void client_service_end(Flag flag) { /* != 0 - raised by signal, ==0 - error */
	short control, last = sharedmem_end();
	write(pdesc[1], &last, sizeof(short));
	read(pdesc2[0], &control, sizeof(short));
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
	char prev_roomname[ROOM_NAME_MAX_LENGTH] = "";
	char buf_login[USER_NAME_MAX_LENGTH] = {'\0'}, buf_room[ROOM_NAME_MAX_LENGTH] = {'\0'};

	if (msgtype == LOGIN) {
		login_data->username[USER_NAME_MAX_LENGTH-1] = '\0';
		while (i < MAX_USERS_NUMBER && room_user_data[i].username[0])
			++i;
		if (i < MAX_USERS_NUMBER) {
			response_data->response_type = (clients < MAX_USERS_NUMBER && add_user_in_shmem(login_data->username, queue_id) != FAIL) ? 
			LOGIN_SUCCESS : LOGIN_FAILED;
			if (response_data->response_type == LOGIN_SUCCESS) {
				strcpy(room_user_data[i].username, login_data->username);
				room_user_data[i].id = login_data->ipc_id;
				clients++;
				sprintf(response_data->content, "%s%s", "Welcome, ", login_data->username);
				send_message(login_data->ipc_id, response_data->type, response_data);
			}
			strcpy(buf_login, login_data->username);
		}
	}
	else if (msgtype == LOGOUT) {
		login_data->username[USER_NAME_MAX_LENGTH-1] = '\0';
		response_data->response_type = (clients > 0 && del_user_in_shmem(login_data->username, queue_id) != FAIL) ?
		LOGOUT_SUCCESS : LOGOUT_FAILED;
		if (response_data->response_type == LOGOUT_SUCCESS) {
			while (i < MAX_USERS_NUMBER && strcmp(room_user_data[i].username, login_data->username))
				++i;
			if (i < MAX_USERS_NUMBER) {
				if (room_for_removal(room_user_data[i].roomname))
					del_room_in_shmem(room_user_data[i].roomname, queue_id);
				strcpy(buf_login, login_data->username);
				room_user_data[i].username[0] = '\0';
				clients--;
				sprintf(response_data->content, "%s - %s", login_data->username, "logout.");
				send_message(login_data->ipc_id, response_data->type, response_data);
			}
		}
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
		room_data->user_name[USER_NAME_MAX_LENGTH-1] = '\0';
		room_data->room_name[ROOM_NAME_MAX_LENGTH-1] = '\0';
		int i = 0, user_index = -1, more_in_room = 0, needs_add = 1;
		if (room_data->operation_type == ENTER_ROOM || room_data->operation_type == CHANGE_ROOM) {
			while (i < MAX_USERS_NUMBER) {
				if (!strcmp(room_user_data[i].username, room_data->user_name)) { /* found the same username */
					strcpy(prev_roomname, room_user_data[i].username);
					if ((room_data->operation_type == ENTER_ROOM && room_user_data[i].roomname[0]) || 
					!strcmp(room_user_data[i].roomname, room_data->room_name)) { /* already in (the same) room */
						user_index = -1;
						i = MAX_USERS_NUMBER;
					}
					else {
						user_index = i;
					}
				}
				else if (!strcmp(room_user_data[i].roomname, room_data->room_name)) /* if there is more users in given room */
					needs_add = 0;
				++i;
			}
			if (room_data->operation_type == CHANGE_ROOM) {
				if (i < MAX_USERS_NUMBER && user_index > -1) { /* user was found and is not in any room */ 
					for (i = 0; i < MAX_USERS_NUMBER; ++i) {
						if (room_user_data[i].roomname[0] && !strcmp(room_user_data[i].roomname, prev_roomname) && i != user_index)
							break;
					}
					if (i == MAX_USERS_NUMBER) { /* previous room should be deleted */
						del_room_in_shmem(room_user_data[user_index].roomname, queue_id);
					}
				}
			}
			if (user_index > -1) {
				if ((needs_add && add_room_in_shmem(room_data->room_name, queue_id) != FAIL) || !needs_add) { /* added in shmem, so adding locally */
					strcpy(room_user_data[user_index].roomname, room_data->room_name);
					response_data->response_type = (room_data->operation_type == CHANGE_ROOM) ? CHANGE_ROOM_SUCCESS : ENTERED_ROOM_SUCCESS;
				}
				else {
					response_data->response_type = (room_data->operation_type == CHANGE_ROOM) ? CHANGE_ROOM_FAILED : ENTERED_ROOM_FAILED;
				}
				send_message(room_user_data[user_index].id, response_data->type, response_data); 
			}
		}
		if (room_data->operation_type == LEAVE_ROOM) {
			while (i < MAX_USERS_NUMBER) {
				if (!strcmp(room_user_data[i].roomname, room_data->room_name) && room_data->room_name[0]) {
					if (!strcmp(room_user_data[i].username, room_data->user_name))
						user_index = i;
				}
				++i;	
			}
			if (user_index > -1) {
				if (room_user_data[user_index].roomname[0]) {
					perror(room_user_data[user_index].roomname);
					if (room_for_removal(room_user_data[user_index].roomname))
						del_room_in_shmem(room_user_data[user_index].roomname, queue_id);
					room_user_data[user_index].roomname[0] = '\0';
					response_data->response_type = LEAVE_ROOM_SUCCESS;
				}
				else
					response_data->response_type = LEAVE_ROOM_FAILED;
				send_message(room_user_data[user_index].id, response_data->type, response_data);
			}
			
		}
	}
	else if (msgtype == MESSAGE) {
		chatmsg_data->sender[USER_NAME_MAX_LENGTH-1] = chatmsg_data->receiver[USER_NAME_MAX_LENGTH-1] = '\0';
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
	inform_log_service((short) response_data->response_type, buf_login, buf_room);
}

short room_for_removal(const char * room) {
	int i = 0, found = 0;
	if (room[0]) {
		for (i = 0; i < MAX_USERS_NUMBER; ++i) {
			if (!strcmp(room_user_data[i].roomname, room))
				++found;
			if (found > 1)
				break;
		}
		return (i < MAX_USERS_NUMBER) ? 1 : 0;
	}
	return 0;
}

void inform_log_service(unsigned short type, const char * user, const char * room) {
	char control_byte;
	write(pdesc[1], &type, sizeof(short));
	read(pdesc2[0], &control_byte, 1);
	write(pdesc[1], user, strlen(user)+1);
	read(pdesc2[0], &control_byte, 1);
	write(pdesc[1], room, strlen(room)+1);
	read(pdesc2[0], &control_byte, 1);
}
