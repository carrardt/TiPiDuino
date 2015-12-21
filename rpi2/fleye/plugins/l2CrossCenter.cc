#include "fleye/cpuworker.h"
#include "fleye/plugin.h"
#include "fleye/render_window.h"
#include "fleye/fbo.h"
#include "fleye/compiledshadercache.h"
#include "fleye/imageprocessing.h"

#include <iostream>

FLEYE_REGISTER_PLUGIN(l2CrossCenter)
FLEYE_REGISTER_GL_DRAW(drawObjectPosition)

static float targetPosX = 0.0f;
static float targetPosY = 0.0f;
static float laserPosX = 0.0f;
static float laserPosY = 0.0f;
static FleyeRenderWindow* render_buffer = 0;

void l2CrossCenter_setup(const ImageProcessingState* ip)
{
	render_buffer = ip->getRenderBuffer("l2c-render-buffer");
	std::cout<<"L2CrossCenter setup : render_buffer @"<<render_buffer<<"\n";
}

#define MINMAX_STAT(x) if(x<x##Min) x##Min=x; else if(x>x##Max) x##Max=x

void l2CrossCenter_run(const ImageProcessingState* ip, CPU_TRACKING_STATE * state)
{
	uint32_t width=0, height=0;
	const uint32_t* base_ptr = (const uint32_t*) read_offscreen_render_window(render_buffer,&width,&height);
	int x,y;
	int obj1_sumx=0,obj1_sumy=0;
	int obj2_sumx=0,obj2_sumy=0;
	int obj1_count=0, obj2_count=0;
	int obj1_L2max=1, obj2_L2max=1;
	
	//std::cout<<"base_ptr="<<base_ptr<<", w="<<width<<", h="<<height<<"\n";
	int u1Min=255;
	int u1Max=0;
	int r1Min=255;
	int r1Max=0;
	int u2Min=255;
	int u2Max=0;
	int r2Min=255;
	int r2Max=0;
	
	
	for(y=0;y<height;y++)
	{
		const uint32_t* p = base_ptr + y * width;
		for(x=0;x<width;x++)
		{
			uint32_t value = *(p++);
			//uint32_t mask1 = ( value & 0x00008080 ) ;
			//uint32_t mask2 = ( value & 0x80800000 ) ;
			uint32_t mask1 = ( value & 0x00808000 ) ;
			uint32_t mask2 = ( value & 0x80000080 ) ;
			if( mask1 )
			{
				int r1 = ( value & 0x007F0000 ) >> 20;
				int u1 = ( value & 0x00007F00 ) >> 12;

				MINMAX_STAT(u1);
				MINMAX_STAT(r1);

				int m1 = (r1>u1) ? r1 : u1;

				if( m1>obj1_L2max ) { obj1_count = obj1_sumx = obj1_sumy = 0; obj1_L2max=m1; }
				else if( m1==obj1_L2max )
				{
					obj1_sumx += x;
					if( r1 >= 1 ) obj1_sumx += 1<<(r1-1);
					obj1_sumy += y;
					if( u1 >= 1 ) obj1_sumy += 1<<(u1-1);
					++ obj1_count;
				}
			}
			if( mask2 )
			{
				int r2 = ( value & 0x0000007F ) >> 4;
				int u2 = ( value & 0x7F000000 ) >> 28;

				MINMAX_STAT(u2);
				MINMAX_STAT(r2);

				int m2 = (r2>u2) ? r2 : u2;
				if( m2>obj2_L2max ) { obj2_count = obj2_sumx = obj2_sumy = 0; obj2_L2max=m2; }
				else if( m2==obj2_L2max )
				{
					obj2_sumx += x;
					if( r2 >= 1 ) obj2_sumx += 1<<(r2-1);
					obj2_sumy += y;
					if( u2 >= 1 ) obj2_sumy += 1<<(u2-1);
					++ obj2_count;
				}
			}
		}
	}

	std::cout<<"u1=["<<u1Min<<';'<<u1Max<<"] r1=["<<r1Min<<';'<<r1Max<<"] => obj1_L2max="<<obj1_L2max<<"\n";
	std::cout<<"u2=["<<u2Min<<';'<<u2Max<<"] r2=["<<r2Min<<';'<<r2Max<<"] => obj2_L2max="<<obj2_L2max<<"\n";

	state->objectCount = 0;
	
	if(obj1_count>0)
	{
		//printf("%d %fx%f\n",count,state->width,state->height);
		state->objectCenter[state->objectCount][0] = (double)obj1_sumx / (double)( obj1_count * width );
		state->objectCenter[state->objectCount][1] = (double)obj1_sumy / (double)( obj1_count * height );
		//std::cout<<"Target position : "<<targetPosX<<","<<targetPosY<<" : count="<<obj1_count<<"\n";
		//printf("%f, %f\n",state->objectCenter[0][0],state->objectCenter[0][1]);

		targetPosX = ( state->objectCenter[state->objectCount][0] - 0.5 ) * 2.0;
		targetPosY = ( state->objectCenter[state->objectCount][1] - 0.5 ) * 2.0;

		state->trackedObjects[ state->objectCount ++ ] = 0;
	}

	if(obj2_count>0)
	{
		//printf("%d %fx%f\n",count,state->width,state->height);
		state->objectCenter[state->objectCount][0] = (double)obj2_sumx / (double)( obj2_count * width );
		state->objectCenter[state->objectCount][1] = (double)obj2_sumy / (double)( obj2_count * height );
		//printf("%f, %f\n",state->objectCenter[0][0],state->objectCenter[0][1]);
		//std::cout<<"Laser position : "<<laserPosX<<","<<laserPosY<<" : count="<<obj2_count<<"\n";

		laserPosX = ( state->objectCenter[state->objectCount][0] - 0.5 ) * 2.0;
		laserPosY = ( state->objectCenter[state->objectCount][1] - 0.5 ) * 2.0;

		state->trackedObjects[ state->objectCount ++ ] = 1;
	}
}

void drawObjectPosition(struct CompiledShaderCache* compiledShader, int pass)
{
	GLfloat varray[24];
	int i;
	for(i=0;i<4;i++)
	{
		int x = i%2;
		int y = ((i/2)+x)%2;
		double ox = x ? -0.05 : 0.05;
		double oy = y ? -0.05 : 0.05;
		varray[i*3+0] = targetPosX +ox;
		varray[i*3+1] = targetPosY +oy;
		varray[i*3+2] = 0.333;
	}
	for(i=4;i<8;i++)
	{
		int x = i%2;
		int y = ((i/2)+x)%2;
		double ox = x ? -0.05 : 0.05;
		double oy = y ? -0.05 : 0.05;
		varray[i*3+0] = laserPosX+ox;
		varray[i*3+1] = laserPosY+oy;
		varray[i*3+2] = 0.0;
	}

	glEnableVertexAttribArray(compiledShader->shader.attribute_locations[0]);
	glVertexAttribPointer(compiledShader->shader.attribute_locations[0], 3, GL_FLOAT, GL_FALSE, 0, varray);
	glDrawArrays(GL_LINES, 0, 8);
	glDisableVertexAttribArray(compiledShader->shader.attribute_locations[0]);
}
