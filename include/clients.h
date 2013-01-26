#ifndef CLIENTS_H
#define CLIENTS_H

#include "libs.h"

#define ROOM_USER 25
#define PING_TIME 1

extern int queue_id;

void client_service();
void client_service_end();
void perform_action(unsigned const int);
short room_for_removal(const char *);
void inform_log_service(unsigned short, const char *, const char *);
#endif
