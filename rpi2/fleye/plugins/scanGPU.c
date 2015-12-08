#include <GLES2/gl2.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "../cpu_tracking.h"

static int memfd = -1;
static const uint32_t memSize = 1<<30;
static const  uint32_t memOffset = 0;
static volatile uint32_t* mappedMem = 0;


#define PATTERN_DWORD 0x004080C0
volatile uint32_t memoryFlag = 0;
	
void scanGPU_setup()
{
	mappedMem = 0;
	memfd = open("/dev/mem", O_RDWR | O_SYNC);
	if( memfd < 0 )
    {
		fprintf(stderr, "scanGPU: Unable to open /dev/mem: %s\n",
		strerror(errno)) ;
		exit(1);
	}
	mappedMem = mmap(NULL, memSize, (PROT_READ | PROT_WRITE), MAP_SHARED, memfd, memOffset);
	if( mappedMem == 0 )
    {
		fprintf(stderr, "scanGPU: Unable to map memory: %s\n",
		strerror(errno)) ;
		exit(1);
	}
	memoryFlag = PATTERN_DWORD;
	printf("scanGPU: fd=%d mapping=%p\n",memfd,mappedMem);
}

void scanGPU_run(CPU_TRACKING_STATE * state)
{
	const uint32_t N = memSize / sizeof(uint32_t);
	const uint32_t searchPattern = 0x004080C0;
	uint32_t seqStart = 0;
	uint32_t seqLen = 0;
	uint32_t i;
	printf("start scanning %u 32bit words @%p...\n",N,mappedMem);
	for(i=0;i<N;i++)
	{
		if( seqLen==0 ) seqStart=i;
		if( mappedMem[i]==searchPattern ) ++seqLen;
		else seqLen=0;
		if( ( i & ((1<<20)-1) ) == 0 ){ printf("%p\n",mappedMem+i); fflush(stdout);}
	}
	printf(" Start=%u Len=%u\n",seqStart,seqLen);
}
