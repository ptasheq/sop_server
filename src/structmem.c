#include "structmem.h"

void * am(const int type, void ** ptr) {
	if (!(*ptr)) {
		switch (type) {
			case LOGIN: case LOGOUT:
				return (*ptr = malloc(sizeof(Msg_login)));
			case REQUEST:
				return (*ptr = malloc(sizeof(Msg_request)));
			case USERS: case ROOMS: case ROOM_USERS:
				return (*ptr = malloc(sizeof(Msg_request_response)));
			default:
				return (void *) 1;
		}
	}
	return (void *) 1;
}
