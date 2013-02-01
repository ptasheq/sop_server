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
Msg_server2server *s2s_data = NULL;
Msg_request_response * request_response_data = NULL;
Room_user * room_user_data = NULL;
int clients = 0;
int queue_id;
int pdesc[2], pdesc2[2], pdesc3[2];

void client_service_init(int * ipcs) { /* it consists of two threads */
	if (pipe(pdesc) == FAIL || pipe(pdesc2) == FAIL || pipe(pdesc3) == FAIL) {
		perror("Couldn't create pipe");
		exit(EXIT_FAILURE);
	}
	
	if ((queue_id = msgget(ipcs[0], 0666 | IPC_CREAT | IPC_EXCL)) == FAIL) {
		perror("Couldn't create message queue.");
		exit(EXIT_FAILURE);
	}

	if (!(clientsrv_pid[0] = fork())) {	/* first thread */
		printf("Queue id: %d\n", queue_id);
		if (sharedmem_init(&ipcs[1]) == FAIL) {
			msgctl(queue_id, IPC_RMID, NULL);
			exit(EXIT_FAILURE);
		}
		if (!allocate_mem(LOGIN, &login_data) || !allocate_mem(RESPONSE, &response_data) || 
			!allocate_mem(ROOM, &room_data) || !allocate_mem(MESSAGE, &chatmsg_data) ||
			!allocate_mem(REQUEST, &request_data) || !allocate_mem(USERS, &request_response_data) || 
			!allocate_mem(ROOM_USER, &room_user_data) || !allocate_mem(SERVER2SERVER, &s2s_data)) {
			perror("Couldn't allocate data structures.");
			client_service_end(0);
		}
		s2s_data->type = SERVER2SERVER;
		short i = 0;
		while (i < MAX_USERS_NUMBER) {
			room_user_data[i].roomname[0] = '\0';
			room_user_data[i].username[0] = '\0';
			room_user_data[i].ping_count = 0;
			++i;
		}
		read(pdesc2[0], &clientsrv_pid[1], sizeof(int));
		write(pdesc[1], &i, sizeof(char));
		write(pdesc[1], &queue_id, sizeof(int));
		read(pdesc2[0], &i, sizeof(char));
		inform_log_service(SERVER_REGISTERED, "", "");
		client_service();
	}
	else if	(clientsrv_pid[0] == FAIL) {
		perror("Couldn't create client thread\n");
		exit(EXIT_FAILURE);
	}

	if (!(clientsrv_pid[1] = fork())) {	/* second thread */
		int sender_id;
		char rcv = 1;
		if (!allocate_mem(SERVER2SERVER, &s2s_data)) {
			perror("Couldn't allocate data structures.");
			exit(EXIT_FAILURE);
		}
		if (fcntl(pdesc3[0], F_SETFL, 0666 | O_NONBLOCK) == FAIL) {
			exit(EXIT_FAILURE);
		}
		set_signal(SIGEND, thread2_end);
		s2s_data->type = SERVER2SERVER;
		perror("started");
		while (1) {
			read(pdesc3[0], &rcv, sizeof(char));
			if (rcv && msgrcv(queue_id, s2s_data, sizeof(Msg_server2server) - sizeof(long), SERVER2SERVER, 0) != FAIL) {
				read(pdesc3[0], &rcv, sizeof(char));
				sender_id = s2s_data->server_ipc_num;
				s2s_data->server_ipc_num = queue_id;
				send_message(sender_id, s2s_data->type, s2s_data);	
			}
			msleep(5);
		}
	}
	else if (clientsrv_pid[1] == FAIL) {
		perror("Couldn't creat client thread\n");
		exit(EXIT_FAILURE);
	}
	char rcv;
	write(pdesc2[1], &clientsrv_pid[1], sizeof(int));
	read(pdesc[0], &rcv, sizeof(char));
}

void client_service() {
	set_signal(SIGEND, client_service_end);
	int i = 0, client;
	time_t t = time(NULL);
	response_data->type = RESPONSE;
	while (1) {
		if (receive_message(queue_id, REQUEST, request_data) != FAIL) {
			if (request_data->request_type == PONG) {
				if ((client = find_user(request_data->user_name)) != FAIL && room_user_data[client].ping_count) {
					--room_user_data[client].ping_count;
					fprintf(stderr, "Ping received\n");
				}
			}
			else { /* some other request */
				perform_action(REQUEST);
			}
		}
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
			perform_action(ROOM);
		}
		if (time(NULL) - t >= PING_TIME && clients) {
			response_data->response_type = PING;
			for (i = 0; i < MAX_USERS_NUMBER; ++i) {
				if (room_user_data[i].username[0]) {
					if (room_user_data[i].ping_count < MAX_FAILS) {
						send_message(room_user_data[i].id, response_data->type, response_data);
						room_user_data[i].ping_count++;
					}
					else {
						del_user_in_shmem(room_user_data[i].username, queue_id);	
						if (users_in_room(room_user_data[i].roomname) == 1)
							del_room_in_shmem(room_user_data[i].roomname, queue_id);
						inform_log_service(100, room_user_data[i].username, "");
						room_user_data[i].username[0] = '\0';
						room_user_data[i].roomname[0] = '\0';
						--clients;
					}
				}
			}
			t = time(NULL);
		}
		msleep(5);
	}
}

void client_service_end(Flag flag) { /* != 0 - raised by signal, ==0 - error */
	short control, last = sharedmem_end();
	perror("doszlo");
	write(pdesc[1], &last, sizeof(short));
	read(pdesc2[0], &control, sizeof(char));
	free_mem(login_data);
	free_mem(response_data);
	free_mem(chatmsg_data);
	free_mem(room_data);
	msgctl(queue_id, IPC_RMID, NULL);
	close(pdesc[1]);
	close(pdesc2[0]);
	if (flag)
		exit(EXIT_SUCCESS);
	else
		exit(EXIT_FAILURE);
}

void perform_action(unsigned const int msgtype) {
	int i = 0;
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
				fprintf(stderr, "zalogowano: %s %d", login_data->username, i);
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
				if (users_in_room(room_user_data[i].roomname) == 1)
					del_room_in_shmem(room_user_data[i].roomname, queue_id);
				strcpy(buf_login, login_data->username);
				room_user_data[i].username[0] = '\0';
				room_user_data[i].roomname[0] = '\0';
				clients--;
				sprintf(response_data->content, "%s - %s", login_data->username, "logout.");
				send_message(login_data->ipc_id, response_data->type, response_data);
			}
		}
	}
	else if (msgtype == REQUEST) {
		response_data->response_type = -1;
		int user_index;
		request_response_data->type = (request_data->request_type == USERS_LIST) ? USERS : 
		(request_data->request_type == ROOMS_LIST) ? ROOMS : ROOM_USERS;
			if ((user_index = find_user(request_data->user_name)) > FAIL && get_list_from_shmem(request_data->request_type, request_response_data) > FAIL) {
				send_message(room_user_data[user_index].id, request_response_data->type, request_response_data);
			}
	}
	else if (msgtype == ROOM) {
		fprintf(stderr, "%s %s", room_data->room_name, room_data->user_name);
		room_data->user_name[USER_NAME_MAX_LENGTH-1] = '\0';
		room_data->room_name[ROOM_NAME_MAX_LENGTH-1] = '\0';
		short i = 0, user_index = -1, needs_add = 1;
		if (room_data->operation_type == ENTER_ROOM || room_data->operation_type == CHANGE_ROOM) {
			while (i < MAX_USERS_NUMBER) {
				if (!strcmp(room_user_data[i].username, room_data->user_name)) { /* found the same username */
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
				if (user_index > -1) { /* user was found and is not in any room */ 
					for (i = 0; i < MAX_USERS_NUMBER; ++i) {
						if (room_user_data[i].roomname[0] && i != user_index && !strcmp(room_user_data[i].roomname, room_user_data[user_index].roomname))
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
			strcpy(buf_room, room_user_data[user_index].roomname);
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
					if (users_in_room(room_user_data[user_index].roomname) == 1) {
						del_room_in_shmem(room_user_data[user_index].roomname, queue_id);
					}
					strcpy(buf_room, room_user_data[user_index].roomname);
					room_user_data[user_index].roomname[0] = '\0';
					response_data->response_type = LEAVE_ROOM_SUCCESS;
				}
				else
					response_data->response_type = LEAVE_ROOM_FAILED;
				send_message(room_user_data[user_index].id, response_data->type, response_data);
			}
			
		}
		strcpy(buf_login, room_user_data[user_index].username);
	}
	else if (msgtype == MESSAGE) {
		response_data->response_type = -1;
		int j = 0, success_flag = 0;	
		chatmsg_data->sender[USER_NAME_MAX_LENGTH-1] = chatmsg_data->receiver[USER_NAME_MAX_LENGTH-1] = '\0';
		i = find_user(chatmsg_data->sender);
		if (chatmsg_data->msg_type == PRIVATE) {
			if ((j = find_user(chatmsg_data->receiver)) > FAIL) { /* receiver is on our server */
				if (send_message(room_user_data[j].id, chatmsg_data->type, chatmsg_data) != FAIL)
					++success_flag;
			} 
		}

		else { /* public message */
			for (; j < MAX_USERS_NUMBER; ++j) {
				if (room_user_data[j].roomname[0] && !strcmp(chatmsg_data->receiver, room_user_data[j].roomname)
				&& strcmp(chatmsg_data->sender, room_user_data[j].username)) { /* user is in room */
					if (send_message(room_user_data[j].id, chatmsg_data->type, chatmsg_data) != FAIL)
						++success_flag;
				}
			}
		}

		if (i > FAIL && !(success_flag && chatmsg_data->msg_type == PRIVATE)) { /* sender is on our server */
			if (send_message_to_servers(chatmsg_data) != FAIL)
				++success_flag;
			response_data->response_type = (success_flag) ? MSG_SEND : MSG_NOT_SEND;
			send_message(room_user_data[i].id, response_data->type, response_data);
		}
		fprintf(stderr, "%d %s", response_data->response_type, chatmsg_data->message);
	}
	if (response_data->response_type > -1 && msgtype != MESSAGE)
		inform_log_service((short) response_data->response_type, buf_login, buf_room);
}

short users_in_room(const char * room) {
	short i = 0, found = 0;
	if (room[0]) {
		for (i = 0; i < MAX_USERS_NUMBER; ++i) {
			if (!strcmp(room_user_data[i].roomname, room))
				++found;
		}
		return found;
	}
	return 0;
}

short find_user(const char * user) {
	short i;
	for (i = 0; i < MAX_USERS_NUMBER; ++i)
		if (!strcmp(user, room_user_data[i].username)) break;
	return (i < MAX_USERS_NUMBER) ? i : FAIL;
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

void thread2_end(int sig) {
		free_mem(s2s_data);
		exit(EXIT_SUCCESS);
}
