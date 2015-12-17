#ifndef __fleye_TextureInput_H_
#define __fleye_TextureInput_H_

#include "fleye/config.h"

#include <string>
#include <vector>

struct GLTexture;

struct TextureInput
{
	std::string uniformName;
	std::vector<GLTexture*> texPool;
};

#endif
