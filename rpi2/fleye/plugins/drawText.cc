#include "fleye/plugin.h"
#include "fleye/compiledshader.h"
#include "fleye/FleyeContext.h"

#include "TextService.h"

#include <iostream>

struct drawText : public FleyePlugin
{	
	void setup(FleyeContext* ctx)
	{
		m_columns = 40;
		m_lines = 25;
		m_txtsvc = TextService_instance();
	}
	
	void draw(FleyeContext* ctx ,CompiledShader* cs,int pass)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glEnableVertexAttribArray(cs->shader.attribute_locations[0]);
		
		for( PositionnedText* pt : m_txtsvc->getPositionnedTexts() )
		{
			int col=0, line=0;
			for ( char c : pt->text )
			{
				if( c == '\n' )
				{
					++line;
					col=0;
				}
				else
				{
					float cx = (pt->x+(col/(float)m_columns))*2.0 - 1.0;
					float cy = 1.0 - (pt->y+(line/(float)m_lines))*2.0 ;
					//std::cout<<"cx="<<cx<<", cy="<<cy<<"\n";
					drawCharPos(cs->shader.attribute_locations[0], cx, cy, c);
					++ col;
					if( col>=80 ) { ++line; col=0; }
				}
			}
		}
		
		glDisableVertexAttribArray(cs->shader.attribute_locations[0]);
		glDisable(GL_BLEND);
	}

	void drawCharPos(GLint vLoc, float posX, float posY, int c)
	{
		GLfloat varray[16];
		for(int i=0;i<4;i++)
		{
			int x = i%2;
			int y = i/2;
			float vx = posX + (x ? 2.0f/m_columns : 0.0f);
			float vy = posY + (y ? 2.0f/m_lines : 0.0f);
			int cy = 15 - c/16;
			int cx = c%16;
			float tx = cx/16.0f + ( x ? 1.0f/16.0f : 0.0f );
			float ty = cy/16.0f + ( y ? 1.0f/16.0f : 0.0f );
			varray[i*4+0] = vx;
			varray[i*4+1] = vy;
			varray[i*4+2] = tx;
			varray[i*4+3] = ty;
		}
		glVertexAttribPointer(vLoc, 4, GL_FLOAT, GL_FALSE, 0, varray);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	void drawChar(GLint vLoc, int col, int line, int c)
	{
		drawCharPos(vLoc, (2.0f*col/(float)m_columns)-1.0f, 1.0f-(2.0f*line/(float)m_lines), c );
	}

	TextService* m_txtsvc;
	int m_columns;
	int m_lines;
};

FLEYE_REGISTER_PLUGIN(drawText);
