#include <libs.h>

int main(int argc, char * argv[]) {
	init(argc, argv);
	io_loop();
	end();
	return 0;
}
