#ifndef LOGFILE_H
#define LOGFILE_H

#include "libs.h"

#define LOGFILE_NAME "server"
#define LOGFILE_EXT ".log"
#define LOGFILE_NAME_LENGTH strlen(LOGFILE_NAME) + strlen(LOGFILE_EXT) + 3

extern int logfile_desc;

void logfile_service_init();
void logfile_service(); 
int lockfile(const int, const unsigned short);
#endif

