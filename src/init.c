#include "init.h"
#include "thread.h"

int clientsrv_pid, logfilesrv_pid;

void init(int argc, char * argv[]) {
	int i, passed_vals[ARG_NUM]; /* 0 - ipc_num, 1,2,3 - shmem_num */
	set_signal(SIGCHLD, end);
	if (argc != 5) {
		perror("Usage: <executable> <ipc_num> <3 x shmem_num>");
		exit(EXIT_FAILURE);
	}
	for (i = 0; i < ARG_NUM; ++i) {
		if ((passed_vals[i] = strtoint(argv[i+1])) == FAIL) {
			perror("Incorrect parameters!\n");
			exit(EXIT_FAILURE);
		}
	}
	client_service_init(passed_vals);
	logfile_service_init();
}

void end(Flag flag) {
	kill(clientsrv_pid, SIGEND); 
	kill(logfilesrv_pid, SIGEND);
	if (flag)
		exit(EXIT_FAILURE);
}

int strtoint(char * str) {
	int i = 0, n = 0, len;
	len = strlen(str);  
    while (str[i] && i < INT_AS_STR_LENGTH) {
 	    if ((str[i] - ASCII_TO_INT) >= 0 && (str[i] - ASCII_TO_INT <= 9)) {
            n += power(10, --len) * (str[i++] - ASCII_TO_INT);
        }
        else {
            n = -1;
            break;
        }
	}
	return n;
}

int power(int base, int exp) {
    int i, res = base;
    if (!exp) {
        return 1;
    }
    for (i = 0; i < exp-1; ++i) {
        res *= base;
    }
    return res;
}

void msleep(unsigned int msec) {
	usleep(1000 * msec);
}

void set_signal(int signum, sa_handler handler) {
	struct sigaction sh;
	sh.sa_handler = handler;
	sigemptyset(&sh.sa_mask);
	sh.sa_flags = 0;
	sigaction(signum, &sh, NULL);
}
