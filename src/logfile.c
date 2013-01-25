#include "init.h"
#include "logfile.h"
#include "thread.h"
#include "protocol.h"
#include <time.h>
#include <sys/sem.h>

int logfile_desc;
int logsem;
char * filename = "/tmp/czat.log";

void logfile_service_init() {
	if (!(logfilesrv_pid = fork())) {
		if ((logsem = semget(SEM_LOGFILE, 1, 0666 | IPC_CREAT | IPC_EXCL)) != FAIL)
			semctl(logsem, 0, SETVAL, 1);
		else if ((logsem = semget(SEM_LOGFILE, 1, 0666 | IPC_CREAT)) == -1) {
			perror("Couldn't create semaphore for logfile.");
			exit(EXIT_FAILURE);
		}
		semctl(logsem, 0, SETVAL, 1);
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
	while (1) {
		prepare_listing(line);
	}
}

void logfile_service_end() {
	char line[TIME_STR_LENGTH + 20];
	short last;
	get_time(line);
	strcpy(&line[TIME_STR_LENGTH], "Server closed.\n");
	semdown(logsem);
	if ((logfile_desc = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0666)) != FAIL) {
		write(logfile_desc, line, strlen(line));
	}
	close(logfile_desc);
	semup(logsem);
	piperead(&last, sizeof(short));
	if (last && !semctl(logsem, 0, GETNCNT))
		semctl(logsem, 0, IPC_RMID);
	close(pdesc[0]);
	close(pdesc2[1]);
	exit(EXIT_SUCCESS);
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
	unsigned short type;
	char rname[ROOM_NAME_MAX_LENGTH];
	char uname[USER_NAME_MAX_LENGTH];
	if (piperead(&type, sizeof(short)) > FAIL) {
		if (piperead(uname, USER_NAME_MAX_LENGTH) > FAIL && piperead(rname, ROOM_NAME_MAX_LENGTH) > FAIL) {
			semdown(logsem);
			get_time(line);
			if ((logfile_desc = open(filename, O_CREAT | O_WRONLY | O_APPEND, 0666)) == FAIL) {
				perror("Couldn't open logfile.");
				exit(EXIT_FAILURE);
			}
			switch (type) {
				case SERVER_REGISTERED: sprintf(&line[TIME_STR_LENGTH], "%s", "Server registered.\n");
				break;
				case LOGIN_SUCCESS: sprintf(&line[TIME_STR_LENGTH], "%s %s %s", "User", uname, "successfully logged in.\n");
				break;
				case LOGIN_FAILED: sprintf(&line[TIME_STR_LENGTH], "%s %s %s", "User", uname, "couldn't log in.\n");
				break;
				case LOGOUT_SUCCESS: sprintf(&line[TIME_STR_LENGTH], "%s %s %s", "User", uname, "successfully logged out.\n");
				break;
				case LOGOUT_FAILED: sprintf(&line[TIME_STR_LENGTH], "%s %s %s", "User", uname, "couldn't log out.\n");
				break;
				case ENTERED_ROOM_SUCCESS: 
					sprintf(&line[TIME_STR_LENGTH], "%s %s %s %s%s", "User", uname, "successfully entered room", rname, ".\n");
				break;
				case ENTERED_ROOM_FAILED: 
					sprintf(&line[TIME_STR_LENGTH], "%s %s %s %s%s", "User", uname, "couldn't enter room", rname, ".\n");
				break;
				case CHANGE_ROOM_SUCCESS: 
					sprintf(&line[TIME_STR_LENGTH], "%s %s %s %s%s", "User", uname, "successfully changed room for", rname, ".\n");
				break;
				case CHANGE_ROOM_FAILED:
					sprintf(&line[TIME_STR_LENGTH], "%s %s %s %s%s", "User", uname, "couldn't change room for", rname, ".\n");
				break;
				case LEAVE_ROOM_SUCCESS:
					sprintf(&line[TIME_STR_LENGTH], "%s %s %s %s%s", "User", uname, "successfully left room", rname, ".\n");
				break;
				case LEAVE_ROOM_FAILED:
					sprintf(&line[TIME_STR_LENGTH], "%s %s %s %s%s", "User", uname, "couldn't leave room", rname, ".\n");
				break;
				default:
					sprintf(&line[TIME_STR_LENGTH], "%s %s%s", "Lost connection with user", uname, ".\n");
			}
			write(logfile_desc, line, strlen(line));
			close(logfile_desc);
			semup(logsem);
		}
		else 
			return FAIL;
	}
	else 
		return FAIL;
	return 0; 
}

short piperead(void * ptr, short bytes) {
	char control_byte = 1;
	if (read(pdesc[0], ptr, bytes) > 0) {
		write(pdesc2[1], &control_byte, 1);
		return 0;
	}
	return FAIL;
}
