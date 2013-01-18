#include "init.h"
#include "logfile.h"
#include "thread.h"
#include "protocol.h"
#include <time.h>
#include <sys/stat.h>

int logfile_desc;

void logfile_service_init() {
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
	set_signal(SIGEND, logfile_service_end);
	get_time(line);
	strcpy(&line[TIME_STR_LENGTH], "Server started.\n");
	write(logfile_desc, line, strlen(line));
	while (1) {
		if (prepare_listing(line) != FAIL)
			write(logfile_desc, line, sizeof(line));
	}
}

void logfile_service_end() {
	char line[TIME_STR_LENGTH + 15];
	get_time(line);
	strcpy(&line[TIME_STR_LENGTH], "Server closed.");
	write(logfile_desc, line, strlen(line));
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

short prepare_listing(char * line) {
	int type;
	char rname[ROOM_NAME_MAX_LENGTH];
	char uname[USER_NAME_MAX_LENGTH];
	if (read(pdesc[0], &type, sizeof(int)) > 0) {
		if (read(pdesc[0], uname, USER_NAME_MAX_LENGTH) > 0 && read(pdesc[0], rname, ROOM_NAME_MAX_LENGTH) > 0) {
			switch (type) {
				case LOGIN_SUCCESS: sprintf(line, "%s %s %s", "User", uname, "successfully logged in.\n");
				break;
				case LOGIN_FAILED: sprintf(line, "%s %s %s", "User", uname, "couldn't log in.\n");
				break;
				case LOGOUT_SUCCESS: sprintf(line, "%s %s %s", "User", uname, "successfully logged out.\n");
				break;
				case LOGOUT_FAILED: sprintf(line, "%s %s %s", "User", uname, "couldn't log out.\n");
				break;
				case ENTERED_ROOM_SUCCESS: 
					sprintf(line, "%s %s %s %s%s", "User", uname, "successfully entered room", rname, ".\n");
				break;
				case ENTERED_ROOM_FAILED: 
					sprintf(line, "%s %s %s %s%s", "User", uname, "couldn't enter room", rname, ".\n");
				break;
				case CHANGE_ROOM_SUCCESS: 
					sprintf(line, "%s %s %s %s%s", "User", uname, "successfully changed room for", rname, ".\n");
				break;
				case CHANGE_ROOM_FAILED:
					sprintf(line, "%s %s %s %s%s", "User", uname, "couldn't change room for", rname, ".\n");
				break;
				case LEAVE_ROOM_SUCCESS:
					sprintf(line, "%s %s %s %s%s", "User", uname, "successfully left room", rname, ".\n");
				break;
				case LEAVE_ROOM_FAILED:
					sprintf(line, "%s %s %s %s%s", "User", uname, "couldn't leave room", rname, ".\n");
				break;
				default:
					sprintf(line, "%s %s%s", "Lost connection with user", uname, ".\n");
			}
		}
		else 
			return FAIL;
	}
	else 
		return FAIL;
	return 0; 
}
