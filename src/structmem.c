#include "structmem.h"

void * am(const int type, void ** ptr) {
	if (!(*ptr)) {
		switch (type) {
			case LOGIN: case LOGOUT:
				return (*ptr = malloc(sizeof(Msg_login)));
			case ROOM:
				return (*ptr = malloc(sizeof(Msg_room)));
			case RESPONSE:
				return(*ptr = malloc(sizeof(Msg_response)));
			case REQUEST:
				return (*ptr = malloc(sizeof(Msg_request)));
			case MESSAGE:
				return (*ptr = malloc(sizeof(Msg_chat_message)));
			case ROOM_USER:
				return (*ptr = malloc(sizeof(Room_user) * MAX_USERS_NUMBER));
			case SERVER2SERVER:
				return (*ptr = malloc(sizeof(Msg_server2server)));
			case USERS: case ROOMS: case ROOM_USERS:
				return (*ptr = malloc(sizeof(Msg_request_response)));
			default:
				return (void *) 1;
		}
	}
	return (void *) 1;
}
