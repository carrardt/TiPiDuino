#ifndef __fleye_TextureInput_H_
#define __fleye_TextureInput_H_

#include "fleye/config.h"

struct RASPITEX_Texture;

struct TextureInput
{
	char uniformName[UNIFORM_NAME_MAX_LEN];
	int poolSize; // number of input textures to cycle through
	RASPITEX_Texture* texPool[MAX_TEXTURES];
};

#endif
