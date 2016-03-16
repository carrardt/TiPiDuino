#include <stdio.h>

int main(int argc, char* argv[])
{
	char text[256];
	unsigned char buf[4];
	int x=0,y=0;
	FILE* fp=0;
	if(argc<2) { fprintf(stderr,"Usage: %s <serial devfice>\n",argv[0]); return 1; }
	fp = fopen(argv[1],"w");
	if(fp==0) { fprintf(stderr,"can't open device\n",argv[1]); return 1; }
		
	while(1)
	{
		printf("X Y : "); fflush(stdout);
		scanf("%d %d",&x,&y);
		buf[0] = (x>>6) | 0x00;
		buf[1] = (x & 0x3F) | 0x40;
		buf[2] = (y>>6) | 0x80;
		buf[3] = (y & 0x3F) | 0xC0;
		printf("%d %d => %02X %02X %02X %02X\n",x,y,buf[0],buf[1],buf[2],buf[3]);
		fwrite(buf,4,1,fp);
		fwrite(buf,4,1,fp);
		fwrite(buf,4,1,fp);
		fwrite(buf,4,1,fp);
	}
	
	
	return 0;
}
