#ifndef fleye_GLTexture_H_
#define fleye_GLTexture_H_

#include <GLES2/gl2.h>

struct GLTexture
{
	GLuint format;
	GLenum target;
	GLuint texid;
	inline GLTexture() : format(GL_RGB), target(GL_TEXTURE_2D), texid(0) {}
};

#endif
