#include "init.h"

key_t ipc_num, shmem_num;
int ipc_id, shmem_id;

void init() {
	if ((ipc_id = msgget(ipc_num, IPC_CREAT | 0666)) == FAIL) {
		perror("Couldn't create a message queue\n");
		exit(EXIT_FAILURE);
	}
}

void end() {

}

void check_parameters(int argc, char * argv[]) {
	if (argc != 3) {
		perror("Usage: <executable> <ipc_num> <shmem_num>");
		exit(EXIT_FAILURE);
	}

	if ((ipc_num = strtoint(argv[1])) == FAIL || (shmem_num = strtoint(argv[2])) == FAIL) {
		perror("Incorrect parameters!\n");
		exit(EXIT_FAILURE);
	}
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
