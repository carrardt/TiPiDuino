#ifndef __fleye_TextureInput_H_
#define __fleye_TextureInput_H_

#include "fleye/config.h"

#ifdef __cplusplus
extern "C" {
#endif

struct RASPITEX_Texture;

struct TextureInput
{
	char uniformName[UNIFORM_NAME_MAX_LEN];
	int poolSize; // number of input textures to cycle through
	struct RASPITEX_Texture* texPool[MAX_TEXTURES];
};

#ifdef __cplusplus
}
#endif

#endif
