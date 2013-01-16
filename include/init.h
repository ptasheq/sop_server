#ifndef INIT_H
#define INIT_H

#include "libs.h"

#define ASCII_TO_INT 48
#define INT_AS_STR_LENGTH 9

extern int clientsrv_pid, logfilesrv_pid;

void init(int, char*[]);
void end();
int strtoint(char *);
int power(int, int);
void msleep(unsigned int);
void client_service_init(int *);
void logfile_service_init();

typedef void (*sa_handler)(int);
void set_signal(int, sa_handler);


#endif
