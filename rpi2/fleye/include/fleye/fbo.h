#ifndef __fleye_FrameBufferObject_H_
#define __fleye_FrameBufferObject_H_

#include <GLES2/gl2.h>

#include <string>

struct GLTexture;
struct ImageProcessingState;

struct FrameBufferObject
{
	GLuint width, height; // dimensions
	GLuint fb; // frame buffer
	GLTexture* texture;
};

int add_fbo(ImageProcessingState* ip, const std::string& name, GLint colorFormat, GLint w, GLint h);

#endif
