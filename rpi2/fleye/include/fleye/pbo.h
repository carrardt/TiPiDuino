#ifndef __FLEYE_PixelBufferObject_h
#define __FLEYE_PixelBufferObject_h

#include <GLES2/gl2.h>

struct PixelBufferObject
{
	GLuint pbo;
	int width, height;
	GLubyte* buffer;
	
	inline PixelBufferObject() : pbo(0), width(0), height(0), buffer(0) {}
	inline void resize(int w, int h)
	{
		width=w;
		height=h;
		if( pbo == 0 )
		{
			glGenBuffers(1,&pbo);
		}
/*		glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
		glBufferData(GL_PIXEL_PACK_BUFFER, width* height * 4, 0, GL_STREAM_READ);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);*/
	}
};

#endif
