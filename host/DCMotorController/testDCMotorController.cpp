#include <cstdint>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

/*
 * echo "7 50 7 50" | ./testdcmotorcontroller /dev/ttyUSB1
 * 
 * reset sequence :
 * echo "0 0 0 0" | ./testdcmotorcontroller /dev/ttyUSB1
 * echo "0 0 0 255" | ./testdcmotorcontroller /dev/ttyUSB1
 */

int main(int argc, char* argv[])
{
	if( argc<2 ) return 1;
	int serial_fd = open( argv[1], O_RDWR | O_SYNC | O_NONBLOCK);
	if( serial_fd < 0 ) return 2;
	struct termios serialConfig;
	tcgetattr(serial_fd,&serialConfig);
	cfmakeraw(&serialConfig);
	cfsetspeed(&serialConfig, 38400 );
	tcsetattr(serial_fd,TCSANOW,&serialConfig);
	
	int s1=60, t1=200, s2=60, t2=200;
	scanf("%d %d %d %d",&s1,&t1,&s2,&t2);	
	printf("send: %d %d %d %d\n",s1,t1,s2,t2);
	uint8_t buf[4] = { s1, t1, s2, t2 };
	write(serial_fd,buf,4);
	fsync(serial_fd);
	
	return 0;
}


