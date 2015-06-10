#include <stdio.h>
#include "gpio.h"

int main(int argc, char* argv[])
{
	if(argc<3) return 1;
	init_gpio();
	gpio_write_xy_i(atoi(argv[1]),atoi(argv[2]));
	return 0;
}
