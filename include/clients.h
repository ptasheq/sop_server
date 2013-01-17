#ifndef CLIENTS_H
#define CLIENTS_H

#include "libs.h"

extern int queue_id;

void client_service();
void client_service_end();
void perform_action(unsigned const int);
#endif
