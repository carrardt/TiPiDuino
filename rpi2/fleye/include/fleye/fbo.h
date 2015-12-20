#ifndef __fleye_FrameBufferObject_H_
#define __fleye_FrameBufferObject_H_

#include <GLES2/gl2.h>

#include <string>

struct GLTexture;
struct ImageProcessingState;
struct FleyeRenderWindow;

struct FrameBufferObject
{
	GLuint width, height; // dimensions
	GLuint fb; // frame buffer
	GLTexture* texture;
	FleyeRenderWindow* render_window;
	inline FrameBufferObject() : width(0), height(0), fb(0), texture(0), render_window(0) {}
};

FrameBufferObject* add_fbo(ImageProcessingState* ip, const std::string& name, GLint colorFormat, GLint w, GLint h);

#endif
