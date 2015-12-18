#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <stdio.h>
#include <string.h>

#include "fleye/cpuworker.h"
#include "fleye/plugin.h"
#include "fleye/config.h"

extern "C" {
#include "thirdparty/tga.h"
}

FLEYE_REGISTER_PLUGIN(imageRecorder)

void imageRecorder_setup()
{
	printf("imageRecorder plugin ready\n");
}

void imageRecorder_run(CPU_TRACKING_STATE * state)
{
	static int count = 0;
	char tmp[128];
	uint32_t imgSize = state->width*state->height*4;
	
	sprintf(tmp,"/tmp/capture%04d.tga",count++);
	FILE* fp = fopen(tmp,"w");
	write_tga(fp, state->width,  state->height, state->image, imgSize);
	fclose(fp);
}
