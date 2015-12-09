#ifndef __fleye_ShaderPass_H_
#define __fleye_ShaderPass_H_

#include <EGL/egl.h>
#include "fleye/config.h"
#include "fleye/texture.h"
#include "fleye/shaderprogram.h"
#include "fleye/fbo.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TextureInput
{
	char uniformName[UNIFORM_NAME_MAX_LEN];
	int poolSize; // number of input textures to cycle through
	RASPITEX_Texture* texPool[MAX_TEXTURES];
} TextureInput;

typedef struct CompiledShaderCache
{
	int textureTargets[SHADER_MAX_INPUT_TEXTURES];
	int samplerUniformLocations[SHADER_MAX_INPUT_TEXTURES];
	RASPITEXUTIL_SHADER_PROGRAM_T shader;
} CompiledShaderCache;

typedef struct ShaderPass
{
	int nInputs;
	TextureInput inputs[SHADER_MAX_INPUT_TEXTURES];
	char* vertexSource;
	char* fragmentSourceWithoutTextures;
	int compileCacheSize;
	CompiledShaderCache shaderCahe[SHADER_COMPILE_CACHE_SIZE];
	int fboPoolSize;
	RASPITEX_FBO* fboPool[MAX_FBOS];
	RASPITEX_Texture* finalTexture;
} ShaderPass;

#ifdef __cplusplus
}
#endif

#endif
