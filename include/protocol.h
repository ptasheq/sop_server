#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "libs.h"

#define USER_NAME_MAX_LENGTH 10
#define RESPONSE_LENGTH 50
#define MAX_SERVERS_NUMBER 15
#define MAX_USERS_NUMBER 20
#define ROOM_NAME_MAX_LENGTH 10

typedef struct {
	char user_name[USER_NAME_MAX_LENGTH];
	int server_id;
} User_server;

typedef struct {
	char room_name[ROOM_NAME_MAX_LENGTH];
	int server_id;
} Room_server;

enum Msg_type {LOGIN=1, RESPONSE, LOGOUT, REQUEST, MESSAGE, ROOM, SERVER2SERVER};

typedef struct {
	long type;
	char username[USER_NAME_MAX_LENGTH];
	key_t ipc_num;
} Msg_login;

enum Response_type {
	LOGIN_SUCCESS=1, LOGIN_FAILED, LOGOUT_SUCCESS, LOGOUT_FAILED,
	MSG_SEND, MSG_NOT_SEND, ENTERED_ROOM_SUCCESS, ENTERED_ROOM_FAILED,
	CHANGE_ROOM_SUCCESS, CHANGE_ROOM_FAILED, LEAVE_ROOM_SUCCESS,
	LEAVE_ROOM_FAILED, PING};

typedef struct {
	long type;
	int response_type;
	char content[RESPONSE_LENGTH];
} Msg_response;

enum Request_type{USERS_LIST=1, ROOMS_LIST, ROOM_USERS_LIST, PONG};
typedef struct{
    long type;
    int request_type;
    char user_name[USER_NAME_MAX_LENGTH];
}Msg_request;

typedef struct{
    long type;
    char users[MAX_SERVERS_NUMBER * MAX_USERS_NUMBER][USER_NAME_MAX_LENGTH];
}Msg_users_list;

enum CHAT_MESSAGE_TYPE {PUBLIC=1, PRIVATE};

typedef struct {
	long type;
	int msg_type;
	char send_time[6];
	char sender[USER_NAME_MAX_LENGTH];
	char receiver[USER_NAME_MAX_LENGTH];
} Msg_chat_message;

enum ROOM_OPERATION_TYPE {ENTER_ROOM=1, LEAVE_ROOM, CHANGE_ROOM};

typedef struct {
	long type;
	int operation_type;
	char user_name [USER_NAME_MAX_LENGTH];
	char room_name [ROOM_NAME_MAX_LENGTH];
} Msg_room;

typedef struct{
    long type;
    int server_ipc_num;
}Msg_server2server;

extern int ipc_id, shmem_id;
extern Msg_login login_data;
extern Msg_response response_data;

int send_message(int, int, ...);
int receive_message(int, int, ...);
#endif
