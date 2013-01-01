#include <libs.h>

int main(int argc, char * argv[]) {
	check_parameters(argc, argv);
	init();
	program_loop();
	end();
	return 0;
}
