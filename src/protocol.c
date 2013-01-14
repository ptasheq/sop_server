#include "protocol.h"
#include <stdarg.h>

int send_message(int id, int msgtype, ...) {
	short i = 0;
	va_list vl;
	va_start(vl, msgtype);
	if (msgtype == LOGIN || msgtype == LOGOUT) {
		Msg_login * msg = va_arg(vl, Msg_login *);
		while (msgsnd(id, msg, USER_NAME_MAX_LENGTH + sizeof(key_t), IPC_NOWAIT) == FAIL && i < MAX_FAILS) {
			++i;
			msleep(5);
		}
	}
	else if (msgtype == RESPONSE) {
		Msg_response * msg = va_arg(vl, Msg_response *);
		while (msgsnd(id, msg, sizeof(int) + RESPONSE_LENGTH, IPC_NOWAIT) == FAIL && i < MAX_FAILS) {
			++i;
			msleep(5);
		}
	}
	else if (msgtype == REQUEST) {
		Msg_request * msg = va_arg(vl, Msg_request *);
		while (msgsnd(id, msg, sizeof(int) + USER_NAME_MAX_LENGTH, IPC_NOWAIT) == FAIL && i < MAX_FAILS) {
            ++i;
            msleep(5);
		}
	}
	else if (msgtype == MESSAGE) {
		Msg_chat_message * msg = va_arg(vl, Msg_chat_message *);
		while (msgsnd(id, msg, sizeof(int) + 6  + STR_BUF_SIZE + 2*USER_NAME_MAX_LENGTH, IPC_NOWAIT) == FAIL && i < MAX_FAILS) {
            ++i;
            msleep(5);
		}
	}
	else if (msgtype == ROOMS || msgtype == USERS || msgtype == ROOM_USERS) {
		Msg_request_response * msg = va_arg(vl, Msg_request_response *);
		while (msgsnd(id, msg, sizeof(char) * MAX_SERVERS_NUMBER * MAX_USERS_NUMBER * USER_NAME_MAX_LENGTH, IPC_NOWAIT) == FAIL && i < MAX_FAILS) {
		++i;
		msleep(5);
		}
	}
	else i = MAX_FAILS;
	va_end(vl);
	return (i<MAX_FAILS) ? 1 : FAIL;
}

int receive_message(int id, int msgtype, ...) {
	va_list vl;
	va_start(vl, msgtype);
	if (msgtype == LOGIN || msgtype == LOGOUT) {
        Msg_login * msg = va_arg(vl, Msg_login *);
        if (msgrcv(id, msg, USER_NAME_MAX_LENGTH + sizeof(key_t), msgtype, IPC_NOWAIT) != FAIL) {
        	return 1;
		}
    }
    else if (msgtype == REQUEST) {
        Msg_request * msg = va_arg(vl, Msg_request *);
        if (msgrcv(id, msg, sizeof(int) + USER_NAME_MAX_LENGTH, msgtype, IPC_NOWAIT) != FAIL) {
        	return 1;
		}
    }
    else if (msgtype == MESSAGE) {
        Msg_chat_message * msg = va_arg(vl, Msg_chat_message *);
        if (msgrcv(id, msg, sizeof(int) + 6  + STR_BUF_SIZE + 2*USER_NAME_MAX_LENGTH, msgtype, IPC_NOWAIT) != FAIL) {
        	return 1;
		}
    }
    else if (msgtype == ROOM) {
        Msg_room * msg = va_arg(vl, Msg_room *);
        if (msgrcv(id, msg, sizeof(int) + USER_NAME_MAX_LENGTH + ROOM_NAME_MAX_LENGTH, msgtype, IPC_NOWAIT) != FAIL) {
        	return 1;
		}
    }	
	va_end(vl);
	return FAIL;
}
