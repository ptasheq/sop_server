#include "clients.h"
#include "protocol.h"

Msg_login login_data = {LOGIN, "", 0};
Msg_response response_data = {RESPONSE, 0, ""};

void client_service() {
	int fv;
	if ((fv = fork())) {
		int client_id;
	    int err_flag;
    	while ((err_flag = msgrcv(ipc_id, &login_data, USER_NAME_MAX_LENGTH + sizeof(key_t), LOGIN, MSG_NOERROR))) { 
			if (err_flag != FAIL) {
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
