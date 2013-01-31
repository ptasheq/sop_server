#ifndef CLIENTS_H
#define CLIENTS_H

#include "libs.h"

#define ROOM_USER 25
#define PING_TIME 1

void client_service();
void client_service_end();
void perform_action(unsigned const int);
short users_in_room(const char *);
short find_user(const char *);
void inform_log_service(unsigned short, const char *, const char *);
void thread2_end(int);
#endif
