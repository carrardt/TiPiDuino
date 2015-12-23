#include "fleye/plugin.h"
#include "fleye/compiledshader.h"
#include "fleye/FleyeContext.h"
#include "fleye/imageprocessing.h"
#include "fleye/cpuworker.h"

#include <iostream>

/*
 * use it with zhue_vs vertex shader
 */

struct drawTrackingPos : public FleyePlugin
{
	void draw(struct FleyeContext* ctx, CompiledShader* compiledShader, int pass)
	{
		CpuWorkerState * state = & ctx->ip->cpu_tracking_state;

		glEnableVertexAttribArray(compiledShader->shader.attribute_locations[0]);
		
		for(int i=0;i<state->objectCount;i++)
		{
			float x = ( state->objectCenter[state->objectCount][0] - 0.5 ) * 2.0 ;
			float y = ( state->objectCenter[state->objectCount][1] - 0.5 ) * 2.0 ;
			float h = i / (float) state->objectCount;
			drawCross(compiledShader,x,y,h);
		}

		glDisableVertexAttribArray(compiledShader->shader.attribute_locations[0]);
	}
	
	static void drawCross(struct CompiledShader* cs, float posx, float posy, float hue)
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

};

FLEYE_REGISTER_PLUGIN(drawTrackingPos);

