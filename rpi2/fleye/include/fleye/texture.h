#ifndef fleye_RASPITEX_Texture_H_
#define fleye_RASPITEX_Texture_H_

#include <GLES2/gl2.h>
#include "fleye/config.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ImageProcessingState;

typedef struct RASPITEX_Texture
{
	char name[TEXTURE_NAME_MAX_LEN];
	GLuint format;
	GLenum target;
	GLuint texid;
} RASPITEX_Texture;

RASPITEX_Texture* get_named_texture(struct ImageProcessingState* ip, const char * name);

#ifdef __cplusplus
}
#endif

#endif
