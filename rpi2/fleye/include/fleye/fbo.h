#ifndef __fleye_FrameBufferObject_H_
#define __fleye_FrameBufferObject_H_

#include <GLES2/gl2.h>

struct RASPITEX_Texture;
struct ImageProcessingState;

struct FrameBufferObject
{
	// name given by texture->name
	GLuint width, height; // dimensions
	GLuint fb; // frame buffer
	RASPITEX_Texture* texture;
} ;

int add_fbo(ImageProcessingState* ip, const char* name, GLint colorFormat, GLint w, GLint h);
FrameBufferObject* get_named_fbo(ImageProcessingState* ip, const char * name);

#endif
