#ifndef fleye_RASPITEX_Texture_H_
#define fleye_RASPITEX_Texture_H_

#include <GLES2/gl2.h>
#include "fleye/config.h"

struct ImageProcessingState;

struct RASPITEX_Texture
{
	char name[TEXTURE_NAME_MAX_LEN];
	GLuint format;
	GLenum target;
	GLuint texid;
};

RASPITEX_Texture* get_named_texture(ImageProcessingState* ip, const char * name);

#endif
