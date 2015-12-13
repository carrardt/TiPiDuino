#ifndef __fleye_FrameBufferObject_H_
#define __fleye_FrameBufferObject_H_

#include <GLES2/gl2.h>

struct RASPITEX_Texture;
struct ImageProcessingState;

typedef struct FrameBufferObject
{
	// name given by texture->name
	GLuint width, height; // dimensions
	GLuint fb; // frame buffer
	struct RASPITEX_Texture* texture;
} FrameBufferObject;

#endif
