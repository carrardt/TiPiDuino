#ifndef fleye_RASPITEX_Texture_H_
#define fleye_RASPITEX_Texture_H_

#include <EGL/egl.h>
#include "fleye/config.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct RASPITEX_Texture
{
	char name[TEXTURE_NAME_MAX_LEN];
	GLuint format;
	GLenum target;
	GLuint texid;
} RASPITEX_Texture;


#ifdef __cplusplus
}
#endif

#endif
