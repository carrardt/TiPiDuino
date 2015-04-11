#include "AvrTL.h"

int main(void)
{
	avrtl::boardInit();
	setup();
	for(;;) loop();
}
