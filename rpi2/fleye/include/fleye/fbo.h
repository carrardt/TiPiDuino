#ifndef __fleye_FrameBufferObject_H_
#define __fleye_FrameBufferObject_H_

#include <GLES2/gl2.h>

#ifdef __cplusplus
extern "C" {
#endif

struct RASPITEX_Texture;
struct ImageProcessingState;

typedef struct FrameBufferObject
{
	// name given by texture->name
	GLuint width, height; // dimensions
	GLuint fb; // frame buffer
	struct RASPITEX_Texture* texture;
} FrameBufferObject;

extern int add_fbo(struct ImageProcessingState* ip, const char* name, GLint colorFormat, GLint w, GLint h);
extern FrameBufferObject* get_named_fbo(struct ImageProcessingState* ip, const char * name);

#ifdef __cplusplus
}
#endif

#endif
