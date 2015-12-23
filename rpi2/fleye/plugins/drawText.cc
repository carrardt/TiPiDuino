#include "fleye/plugin.h"
#include "fleye/compiledshader.h"
#include "fleye/FleyeContext.h"
#include <iostream>

struct drawText : public FleyePlugin
{

	void draw(FleyeContext* ctx ,CompiledShader* cs,int pass)
	{
		std::string text = ctx->vars["FPS"] + " FPS";
		int col = 5;
		int line = 5;
		glEnableVertexAttribArray(cs->shader.attribute_locations[0]);
		for( char c : text )
		{
			drawChar(cs->shader.attribute_locations[0], col++, line, c);
		}
		glDisableVertexAttribArray(cs->shader.attribute_locations[0]);
	}

	static void drawCharPos(GLint vLoc, float posX, float posY, int c)
	{
		GLfloat varray[16];
		for(int i=0;i<4;i++)
		{
			int x = i%2;
			int y = i/2;
			float vx = posX + (x ? 1.0/40.0 : 0.0);
			float vy = posY + (y ? 1.0/25.0 : 0.0);
			int cy = 15 - c/16;
			int cx = c%16;
			float tx = cx/16.0 + ( x ? 1.0/16.0 : 0.0 );
			float ty = cy/16.0 + ( y ? 1.0/16.0 : 0.0 );
			varray[i*4+0] = vx;
			varray[i*4+1] = vy;
			varray[i*4+2] = tx;
			varray[i*4+3] = ty;
		}
		glVertexAttribPointer(vLoc, 4, GL_FLOAT, GL_FALSE, 0, varray);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	static void drawChar(GLint vLoc, int col, int line, int c)
	{
		drawCharPos(vLoc, (col/40.0)-1.0, 1.0-(line/25.0), c );
	}

};

FLEYE_REGISTER_PLUGIN(drawText);
