#include "clients.h"
#include "protocol.h"

Msg_login login_data = {LOGIN, "", 0};
Msg_response response_data = {RESPONSE, 0, ""};
Msg_room room_data = {ROOM, 0, "", ""};
Msg_chat_message chatmsg_data = {MESSAGE, 0, "", "", "", ""};

void client_service() {
	int fv;
	if ((fv = fork())) {
		int client_id;
	    int err_flag;
    	while (1) {
			if (receive_message(ipc_id, LOGIN, &login_data) != FAIL) {
				perror("login"); /* not error, but it won't be displayed otherwise */
				if (add_user(login_data.username) != FAIL && (client_id = msgget(login_data.ipc_num, 0666)) != FAIL) {
                    perror(login_data.username);
                    response_data.response_type = LOGIN_SUCCESS;
                    sprintf(response_data.content, "%s%s", "Welcome, ", login_data.username);
                    if (send_message(client_id, response_data.type, &response_data)) {
                    
                    }
                }
                else {
                    response_data.response_type = LOGIN_FAILED;
                    send_message(client_id, response_data.type, &response_data);    
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
			else {
				msleep(10);
			}

        }
    	
	}
	else if	(fv == FAIL) {
		perror("Couldn't create client thread\n");
		exit(EXIT_FAILURE);
	}
}

int add_user(char * usrname) {
	if (1) {
		return 1;
	}
	else {
		return FAIL;
	}
}
