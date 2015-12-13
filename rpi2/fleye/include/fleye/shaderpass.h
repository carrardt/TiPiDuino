#ifndef __fleye_ShaderPass_H_
#define __fleye_ShaderPass_H_

#include "fleye/config.h"
#include "fleye/compiledshadercache.h"
#include "fleye/textureinput.h"

struct RASPITEX_Texture;
struct FrameBufferObject;

struct ShaderPass
{
	int nInputs;
	TextureInput inputs[SHADER_MAX_INPUT_TEXTURES];
	char* vertexSource;
	char* fragmentSourceWithoutTextures;
	int compileCacheSize;
	CompiledShaderCache shaderCahe[SHADER_COMPILE_CACHE_SIZE];
	int fboPoolSize;
	FrameBufferObject* fboPool[MAX_FBOS];
	RASPITEX_Texture* finalTexture;
};

typedef struct ShaderPass ShaderPass;

#endif
