#ifndef LOGFILE_H
#define LOGFILE_H

#include "libs.h"

#define LOGFILE_NAME "server"
#define LOGFILE_EXT ".log"
#define LOGFILE_NAME_LENGTH strlen(LOGFILE_NAME) + strlen(LOGFILE_EXT) + 3
#define TIME_STR_LENGTH 14
#define SEC_IN_DAY 86400

extern int logfile_desc;

void logfile_service_init();
void logfile_service(); 
void logfile_service_end();
char correct_logfile(char *);
void get_time(char *);
#endif

