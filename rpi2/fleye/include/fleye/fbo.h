#ifndef __fleye_RASPITEX_FBO_H_
#define __fleye_RASPITEX_FBO_H_

#include <EGL/egl.h>
#include "fleye/texture.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RASPITEX_FBO
{
	// name given by texture->name
	GLuint width, height; // dimensions
	GLuint fb; // frame buffer
	RASPITEX_Texture* texture;
} RASPITEX_FBO;

#ifdef __cplusplus
}
#endif

#endif
