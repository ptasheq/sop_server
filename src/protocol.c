#include "protocol.h"
#include <stdarg.h>

int send_message(int id, int msgtype, ...) {
	short i = 0;
	va_list vl;
	va_start(vl, msgtype);
	if (msgtype == LOGIN || msgtype == LOGOUT) {
		Msg_login * msg = va_arg(vl, Msg_login *);
		while (msgsnd(id, msg, sizeof(Msg_login) - sizeof(long), IPC_NOWAIT) == FAIL && i < MAX_FAILS) {
			++i;
			msleep(5);
		}
	}
	else if (msgtype == RESPONSE) {
		Msg_response * msg = va_arg(vl, Msg_response *);
		while (msgsnd(id, msg, sizeof(Msg_response) - sizeof(long), IPC_NOWAIT) == FAIL && i < MAX_FAILS) {
			++i;
			msleep(5);
		}
	}
	else if (msgtype == REQUEST) {
		Msg_request * msg = va_arg(vl, Msg_request *);
		while (msgsnd(id, msg, sizeof(Msg_request) - sizeof(long), IPC_NOWAIT) == FAIL && i < MAX_FAILS) {
            ++i;
            msleep(5);
		}
	}
	else if (msgtype == MESSAGE) {
		Msg_chat_message * msg = va_arg(vl, Msg_chat_message *);
		while (msgsnd(id, msg, sizeof(Msg_chat_message) - sizeof(long), IPC_NOWAIT) == FAIL && i < MAX_FAILS) {
            ++i;
            msleep(5);
		}
	}
	else if (msgtype == SERVER2SERVER) {
		Msg_server2server * msg = va_arg(vl, Msg_server2server *);
		while (msgsnd(id, msg, sizeof(Msg_server2server) - sizeof(long), IPC_NOWAIT) == FAIL && i < MAX_FAILS) {
			++i;
			msleep(5);
		}
	}
	else if (msgtype == ROOMS || msgtype == USERS || msgtype == ROOM_USERS) {
		Msg_request_response * msg = va_arg(vl, Msg_request_response *);
		while (msgsnd(id, msg, sizeof(Msg_request_response) - sizeof(long), IPC_NOWAIT) == FAIL && i < MAX_FAILS) {
		++i;
		msleep(5);
		}
	}
	else i = MAX_FAILS;
	va_end(vl);
	return (i<MAX_FAILS) ? 1 : FAIL;
}

int receive_message(int id, int msgtype, ...) {
	short i = 0;
	va_list vl;
	va_start(vl, msgtype);
	if (msgtype == LOGIN || msgtype == LOGOUT) {
        Msg_login * msg = va_arg(vl, Msg_login *);
        i = msgrcv(id, msg, sizeof(Msg_login) - sizeof(long), msgtype, IPC_NOWAIT);
    }
    else if (msgtype == REQUEST) {
        Msg_request * msg = va_arg(vl, Msg_request *);
        i = msgrcv(id, msg, sizeof(Msg_request) - sizeof(long), msgtype, IPC_NOWAIT);
    }
    else if (msgtype == MESSAGE) {
        Msg_chat_message * msg = va_arg(vl, Msg_chat_message *);
        i = msgrcv(id, msg, sizeof(Msg_chat_message) - sizeof(long), msgtype, IPC_NOWAIT);
    }
    else if (msgtype == ROOM) {
        Msg_room * msg = va_arg(vl, Msg_room *);
        i = msgrcv(id, msg, sizeof(Msg_room) - sizeof(long), msgtype, IPC_NOWAIT); 
    }	
	else if (msgtype == SERVER2SERVER) {
		Msg_server2server * msg = va_arg(vl, Msg_server2server *);
		i = msgrcv(id, msg, sizeof(Msg_server2server) - sizeof(long), msgtype, IPC_NOWAIT);
	}
	va_end(vl);
	return i;
}
