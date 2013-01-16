#include "init.h"
#include "logfile.h"

int logfile_desc = FAIL;

void logfile_service_init() {
	if (!(logfilesrv_pid = fork())) {
		int i = 1;
		char filename[LOGFILE_NAME_LENGTH];
		sprintf(filename, "%s%d%s", LOGFILE_NAME, i, LOGFILE_EXT);
		while (i < 100 && lockfile((logfile_desc = creat(filename, 0666)), 0) == FAIL) { /* while logfile with chosen name is already open */
			if (access(filename, F_OK) == FAIL) { /* file couldn't be created */
				return;
			}
			++i;
			sprintf(filename, "%s%d%s", LOGFILE_NAME, i, LOGFILE_EXT);
		}
	}
	else if (logfilesrv_pid == FAIL) {
		perror("Couldn't create logfile thread.");
		exit(EXIT_FAILURE);
	}
}

void logfile_service() {

}

void logfile_service_end() {
	lockfile(logfile_desc, 1);
	exit(EXIT_SUCCESS);
}

int lockfile(const int fd, const unsigned short flag) {
	/*flag = 0 -> locksotherwise -> unlocks */
	struct flock lock;
	lock.l_type = (flag == 0) ? F_WRLCK : F_UNLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_pid = 0;
	lock.l_len = 0; /* we want to lock all bytes */
    return fcntl(fd, F_SETLK, &lock);
}
