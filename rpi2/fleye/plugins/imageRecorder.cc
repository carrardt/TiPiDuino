
#include "fleye/cpuworker.h"
#include "fleye/plugin.h"
#include "fleye/config.h"
#include "fleye/render_window.h"
#include "fleye/fbo.h"
#include "fleye/imageprocessing.h"

#include "thirdparty/tga.h"

#include <GLES2/gl2.h>
#include <iostream>

FLEYE_REGISTER_PLUGIN(imageRecorder)

static FleyeRenderWindow* render_buffer = 0;

void imageRecorder_setup(const ImageProcessingState* ip)
{
	render_buffer = ip->getRenderBuffer("video");
	std::cout<<"imageRecorder setup : render_buffer @"<<render_buffer<<"\n";
}

void imageRecorder_run(const ImageProcessingState* ip, CPU_TRACKING_STATE * state)
{
	static int count = 0;
	char tmp[128];
	uint32_t width=0, height=0;
	uint8_t* image = (uint8_t*) read_offscreen_render_window(render_buffer,&width,&height);
	uint32_t imgSize = width * height * 4;
	for(uint32_t i=0;i<(width * height);i++)
	{
		uint8_t b = image[i*4+0];
		uint8_t g = image[i*4+1];
		uint8_t r = image[i*4+2];
		uint8_t a = image[i*4+3];
		image[i*4+0] = r;
		image[i*4+1] = g;
		image[i*4+2] = b;
		image[i*4+3] = a;
	}
	
	sprintf(tmp,"/tmp/capture%04d.tga",count++);
	std::cout<<"write image @"<<(void*)image<<" ("<<width<<'x'<<height<<") to "<<tmp<<"\n";
	FILE* fp = fopen(tmp,"w");
	write_tga(fp, width,  height, image, imgSize);
	fclose(fp);
}
