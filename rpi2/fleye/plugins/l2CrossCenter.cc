#include "fleye/cpuworker.h"
#include "fleye/plugin.h"
#include "fleye/FleyeRenderWindow.h"
#include "fleye/fbo.h"
#include "fleye/compiledshadercache.h"
#include "fleye/imageprocessing.h"
#include "fleye/FleyeContext.h"

#include <iostream>

FLEYE_REGISTER_PLUGIN(l2CrossCenter)
FLEYE_REGISTER_GL_DRAW(drawObjectPosition)

static float targetPosX = 0.0f;
static float targetPosY = 0.0f;
static float laserPosX = 0.0f;
static float laserPosY = 0.0f;

static FleyeRenderWindow* render_buffer = 0;

void l2CrossCenter_setup(FleyeContext* ctx)
{
	render_buffer = ctx->ip->getRenderBuffer("l2c-render-buffer");
	std::cout<<"L2CrossCenter setup : render_buffer @"<<render_buffer<<"\n";
}

#if 1
#define DECLARE_MINMAX_STAT(x) uint32_t x##Min=256, x##Max=0
#define UPDATE_MINMAX_STAT(v) if(v<v##Min) v##Min=v; else if(v>v##Max) v##Max=v
#define PRINT_MINMAX_STAT(x) std::cout<<" "<<#x<<"=["<<x##Min<<';'<<x##Max<<']';
#else
#define DECLARE_MINMAX_STAT(x) do{}while(0)
#define UPDATE_MINMAX_STAT(v) do{}while(0)
#define PRINT_MINMAX_STAT(x) do{}while(0)
#endif

void l2CrossCenter_run(FleyeContext* ctx)
{
	CPU_TRACKING_STATE * state = & ctx->ip->cpu_tracking_state;
	int width=0, height=0;
	const uint32_t* base_ptr = (const uint32_t*) render_buffer->readBack(width,height);
	uint32_t obj1_sumx=0,obj1_sumy=0;
	uint32_t obj2_sumx=0,obj2_sumy=0;
	uint32_t obj1_count=0, obj2_count=0;
	uint32_t obj1_L2max=1, obj2_L2max=1;
	
	DECLARE_MINMAX_STAT(u1);
	DECLARE_MINMAX_STAT(r1);
	DECLARE_MINMAX_STAT(u2);
	DECLARE_MINMAX_STAT(r2);
	
	DECLARE_MINMAX_STAT(R);
	DECLARE_MINMAX_STAT(G);
	DECLARE_MINMAX_STAT(B);

	for(uint32_t y=0;y<height;y++)
	{
		const uint32_t* p = base_ptr + y * width;
		for(uint32_t x=0;x<width;x++)
		{
			uint32_t value = *(p++);
			uint32_t R = ( value ) & 0x000000FF;
			uint32_t G = ( value >> 8) & 0x000000FF;
			uint32_t B = ( value >> 16) & 0x000000FF;
			//uint32_t A = ( value >> 24) & 0x000000FF;
			
			UPDATE_MINMAX_STAT(R);
			UPDATE_MINMAX_STAT(G);
			UPDATE_MINMAX_STAT(B);

			if( B != 0 )
			{
				if( B<128 )
				{
					uint32_t r1 = R / 8;
					uint32_t u1 = G / 8;
					uint32_t m1 = (r1>u1) ? r1 : u1;
					if( m1 > obj1_L2max )
					{ 
						obj1_count = 0;
						obj1_sumx = 0;
						obj1_sumy = 0;
						obj1_L2max = m1;
					}
					if( m1 == obj1_L2max )
					{
						obj1_sumx += x;
						//if( r1 >= 1 ) obj1_sumx -= 1<<(r1-1);
						obj1_sumy += y;
						//if( u1 >= 1 ) obj1_sumy -= 1<<(u1-1);
						++ obj1_count;
					}
					UPDATE_MINMAX_STAT(u1);
					UPDATE_MINMAX_STAT(r1);
				}
				else
				{
					uint32_t r2 = R / 8;
					uint32_t u2 = G / 8;
					uint32_t m2 = (r2>u2) ? r2 : u2;
					if( m2 > obj2_L2max )
					{ 
						obj2_count = 0;
						obj2_sumx = 0;
						obj2_sumy = 0; 
						obj2_L2max=m2; 
					}
					if( m2 == obj2_L2max )
					{
						obj2_sumx += x;
						//if( r2 >= 1 ) obj2_sumx -= 1<<(r2-1);
						obj2_sumy += y;
						//if( u2 >= 1 ) obj2_sumy -= 1<<(u2-1);
						++ obj2_count;
					}
					UPDATE_MINMAX_STAT(u2);
					UPDATE_MINMAX_STAT(r2);
				}
			}
		}
	}

	state->objectCount = 0;
		
	if(obj1_count>0)
	{
		obj1_sumx /= obj1_count;
		obj1_sumy /= obj1_count;	
		state->objectCenter[state->objectCount][0] = ((float)obj1_sumx) / (float)width;
		state->objectCenter[state->objectCount][1] = 1.0 - ((float)obj1_sumy) / (float)height;

		targetPosX = ( state->objectCenter[state->objectCount][0] -0.5 ) * 2.0;
		targetPosY = ( state->objectCenter[state->objectCount][1] -0.5 ) * 2.0;

		state->trackedObjects[ state->objectCount ++ ] = 0;
	}

	if(obj2_count>0)
	{
		obj2_sumx /= obj2_count;
		obj2_sumy /= obj2_count;	
		state->objectCenter[state->objectCount][0] = ((float)obj2_sumx) / (float)width;
		state->objectCenter[state->objectCount][1] = 1.0 - ((float)obj2_sumy) / (float)height;

		laserPosX = ( state->objectCenter[state->objectCount][0] - 0.5 ) * 2.0;
		laserPosY = ( state->objectCenter[state->objectCount][1] - 0.5 ) * 2.0;

		state->trackedObjects[ state->objectCount ++ ] = 1;
	}

	if( ctx->frameCounter%30 == 0 )
	{
		PRINT_MINMAX_STAT(R);
		PRINT_MINMAX_STAT(G);
		PRINT_MINMAX_STAT(B);
		PRINT_MINMAX_STAT(u1);
		PRINT_MINMAX_STAT(r1);
		PRINT_MINMAX_STAT(u2);
		PRINT_MINMAX_STAT(r2);
		std::cout<<" c1="<<obj1_count<<" p="<<obj1_sumx<<','<<obj1_sumy;
		std::cout<<" c2="<<obj2_count<<" p="<<obj2_sumx<<','<<obj2_sumy;
		std::cout<<" T="<<targetPosX<<','<<targetPosY;
		std::cout<<" L="<<laserPosX<<','<<laserPosY<<"\n";
	}
}

static void drawCross(struct CompiledShaderCache* cs, float posx, float posy, float hue)
{
	GLfloat varray[12];
	for(int i=0;i<4;i++)
	{
		int x = i%2;
		int y = ((i/2)+x)%2;
		double ox = x ? -0.05 : 0.05;
		double oy = y ? -0.05 : 0.05;
		varray[i*3+0] = posx +ox;
		varray[i*3+1] = posy +oy;
		varray[i*3+2] = hue;
	}
	glVertexAttribPointer(cs->shader.attribute_locations[0], 3, GL_FLOAT, GL_FALSE, 0, varray);
	glDrawArrays(GL_LINES, 0, 4);
}

void drawObjectPosition(struct CompiledShaderCache* compiledShader, int pass)
{
	glEnableVertexAttribArray(compiledShader->shader.attribute_locations[0]);
	
	drawCross(compiledShader,targetPosX,targetPosY,0.333);
	drawCross(compiledShader,laserPosX,laserPosY,0.0);

	glDisableVertexAttribArray(compiledShader->shader.attribute_locations[0]);
}
