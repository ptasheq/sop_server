#ifndef LOGFILE_H
#define LOGFILE_H

#include "libs.h"

#define TIME_STR_LENGTH 14
#define SEM_LOGFILE 38

extern int logfile_desc;
extern int logsem;
extern char * filename;

void logfile_service_init();
void logfile_service(); 
void logfile_service_end();
void get_time(char *);
short prepare_listing(char *);
short piperead(void *, short);
#endif

