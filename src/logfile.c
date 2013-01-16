#include "init.h"
#include "logfile.h"
#include "thread.h"
#include <time.h>
#include <sys/stat.h>

int logfile_desc;
int pdesc[2];

void logfile_service_init() {
	if (pipe(pdesc) == FAIL) {
		perror("Couldn't create pipe");
		exit(EXIT_FAILURE);
	}
	if (!(logfilesrv_pid = fork())) {
		int i = 1;
		char *  filename = malloc(LOGFILE_NAME_LENGTH * sizeof(char));
		sprintf(filename, "%s%d%s", LOGFILE_NAME, i, LOGFILE_EXT);
		while (i < 100 && correct_logfile(filename) == FAIL) {
			++i;
			sprintf(filename, "%s%d%s", LOGFILE_NAME, i, LOGFILE_EXT);
		}
		free(filename);
		logfile_service();	
	}
	else if (logfilesrv_pid == FAIL) {
		perror("Couldn't create logfile thread.");
		exit(EXIT_FAILURE);
	}
}

void logfile_service() {
	char line[STR_BUF_SIZE];
	int n;
	set_signal(SIGEND, logfile_service_end);
	get_time(line);
	strcpy(&line[TIME_STR_LENGTH], "Server started.\n");
	write(logfile_desc, line, strlen(line));
	while (1) {
		if ((n = read(pdesc[0], &line[TIME_STR_LENGTH], STR_BUF_SIZE -TIME_STR_LENGTH-1)) > 0) {
			get_time(line);
			line[n] = '\n';
			write(logfile_desc, line, n+1);
		}
	}
}

void logfile_service_end() {
	close(pdesc[0]);
	close(logfile_desc);
	exit(EXIT_SUCCESS);
}

char correct_logfile(char * filename) {
	struct stat file_stat;
	if (access(filename, F_OK) != FAIL) {
		if (stat(filename, &file_stat) != FAIL && time(NULL) - file_stat.st_atime > SEC_IN_DAY)
			if ((logfile_desc = creat(filename, 0666)) != FAIL)
				return 1;
	}
	else if ((logfile_desc = creat(filename, 0666 | O_EXCL)) != FAIL)
		return 1;
	return FAIL;
}

void get_time(char * str) { /* all numbers are set because of result of ctime (char *) */
    time_t curtime;
    char buf[30];
    short i = 0, spaces = 0;
    if (time(&curtime) != -1) {
        strncpy(buf, ctime(&curtime), 29);
        while (buf[i] && spaces < 1) {
            if (buf[i] == ' ') {
                ++spaces;
            }
            ++i;
        }
        if (spaces == 1){
            strncpy(str, &buf[i], TIME_STR_LENGTH -2);
            str[TIME_STR_LENGTH-2] = ':';
			str[TIME_STR_LENGTH-1] = ' ';
			return;
        }
    }
    strcpy(str, "            ");
}
