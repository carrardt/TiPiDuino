#ifndef __fleye_ShaderPass_H_
#define __fleye_ShaderPass_H_

#include "fleye/config.h"
#include "fleye/compiledshadercache.h"
#include "fleye/textureinput.h"

struct RASPITEX_Texture;
struct RASPITEX_FBO;

#ifdef __cplusplus
extern "C" {
#endif

struct ShaderPass
{
	int nInputs;
	struct TextureInput inputs[SHADER_MAX_INPUT_TEXTURES];
	char* vertexSource;
	char* fragmentSourceWithoutTextures;
	int compileCacheSize;
	struct CompiledShaderCache shaderCahe[SHADER_COMPILE_CACHE_SIZE];
	int fboPoolSize;
	struct RASPITEX_FBO* fboPool[MAX_FBOS];
	struct RASPITEX_Texture* finalTexture;
};

typedef struct ShaderPass ShaderPass;

#ifdef __cplusplus
}
#endif

#endif
