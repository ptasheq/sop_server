#include "ioloop.h"

void io_loop() {
	char buf[IO_BUF_SIZE];
	int n;
	while ((n = read(STDIN_FILENO, buf, IO_BUF_SIZE-1)) > 0 && !wants_exit(buf)) {
		buf[n] = '\0';
		/* Maybe some additional functions here */
	}
}
