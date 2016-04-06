#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define CLOCK_LOW 0x00
#define CLOCK_HIGH 0x80
#define RS_ADDR 0x40
#define RS_DATA 0x00

int main(int argc, char* argv[])
{
	char text[256];
	unsigned char buf[4];
	uint32_t x=1,y=0;
	double t=0.0;
	int i;
	FILE* fp=0;

	if(argc<2) { fprintf(stderr,"Usage: %s <serial devfice>\n",argv[0]); return 1; }
	fp = fopen(argv[1],"w");
	if(fp==0) { fprintf(stderr,"can't open device\n",argv[1]); return 1; }
	
	while( x!=0 )
	{
		//printf("Value : "); fflush(stdout);
		//scanf("%d",&x);
		x = sin(t) * 800.0 + 1200.0;
		t += 0.01;
		
		buf[0] = CLOCK_LOW | RS_ADDR | 0x01;
		buf[1] = CLOCK_LOW | RS_DATA | ( (x>>6)&0x3F );
		buf[2] = CLOCK_HIGH | RS_ADDR | 0x02;
		buf[3] = CLOCK_HIGH | RS_DATA | ( x&0x3F );
		printf("%d => %02X %02X %02X %02X\n",x,buf[0],buf[1],buf[2],buf[3]);
		// 	at 57600 Bauds, this transmission lasts enough to cover a full 10ms pwm cycle,
		// thus guarantees it to be received by peer.
		for(i=0;i<14;i++) { fwrite(buf,4,1,fp); }
		fflush(fp);
	}
	
	fclose(fp);
	return 0;
}
