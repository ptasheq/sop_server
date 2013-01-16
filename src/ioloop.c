#include "ioloop.h"

void io_loop() {
	char buf[IO_BUF_SIZE];
	int n;
	write(STDOUT_FILENO, "Server started.", 15);
	write(STDIN_FILENO, "\n", 1); /*cleaning input from arguments */
	while ((n = read(STDIN_FILENO, buf, IO_BUF_SIZE-1)) > 0) {
		buf[n-1] = '\0';
		if (wants_exit(buf))
			break;
		/* Maybe some additional functions here */
	}
}
