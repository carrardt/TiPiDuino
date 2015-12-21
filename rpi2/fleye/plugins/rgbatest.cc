#include "fleye/cpuworker.h"
#include "fleye/plugin.h"
#include "fleye/render_window.h"
#include "fleye/fbo.h"
#include "fleye/compiledshadercache.h"
#include "fleye/imageprocessing.h"

#include <iostream>

FLEYE_REGISTER_PLUGIN(rgbatest)

static FleyeRenderWindow* render_buffer = 0;

void rgbatest_setup(const ImageProcessingState* ip)
{
	render_buffer = ip->getRenderBuffer("rgbatest");
	std::cout<<"rgbatest setup : render_buffer @"<<render_buffer<<"\n";
}

#define DECLARE_MINMAX_STAT(x) uint32_t x##Min=256, x##Max=0, x##MinX=128, x##MinY=128, x##MaxX=128, x##MaxY=128;

#define UPDATE_MINMAX_STAT(v,x,y) \
	if(v<v##Min) { \
		v##Min=v; \
		v##MinX=x; \
		v##MinY=y; \
	} else if(v>v##Max) { \
		v##Max=v; \
		v##MaxX=x; \
		v##MaxY=y; \
	}
#define PRINT_MINMAX_STAT(x) \
	std::cout<<#x<<" : min="<<x##Min<<" @"<<x##MinX<<','<<x##MinY<<" max="<<x##Max<<" @"<<x##MaxX<<','<<x##MaxY<<"\n";

void rgbatest_run(const ImageProcessingState* ip, CPU_TRACKING_STATE * state)
{
	uint32_t width=0, height=0;
	const uint32_t* base_ptr = (const uint32_t*) read_offscreen_render_window(render_buffer,&width,&height);
	
	//std::cout<<"base_ptr="<<base_ptr<<", w="<<width<<", h="<<height<<"\n";
	DECLARE_MINMAX_STAT(r);
	DECLARE_MINMAX_STAT(g);
	DECLARE_MINMAX_STAT(b);
	DECLARE_MINMAX_STAT(a);
	
	for(uint32_t y=0;y<height;y++)
	{
		for(uint32_t x=0;x<width;x++)
		{
			const uint32_t* p = base_ptr + y * width + x;
			const uint32_t value = *p;
			uint32_t r = ( value ) & 0x000000FF;
			uint32_t g = ( value >> 8) & 0x000000FF;
			uint32_t b = ( value >> 16) & 0x000000FF;
			uint32_t a = ( value >> 24) & 0x000000FF;

			UPDATE_MINMAX_STAT(r,x,y);
			UPDATE_MINMAX_STAT(g,x,y);
			UPDATE_MINMAX_STAT(b,x,y);
			UPDATE_MINMAX_STAT(a,x,y);
		}
	}
	std::cout<<"size="<<width<<'x'<<height<<"\n";
	PRINT_MINMAX_STAT(r);
	PRINT_MINMAX_STAT(g);
	PRINT_MINMAX_STAT(b);
	PRINT_MINMAX_STAT(a);
}
