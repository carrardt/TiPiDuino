#ifndef fleye_CompiledShaderCache_H_
#define fleye_CompiledShaderCache_H_

#include "fleye/shaderprogram.h"

struct RASPITEX_Texture;
struct ShaderPass;

#ifdef __cplusplus
extern "C" {
#endif

struct CompiledShaderCache
{
	int textureTargets[SHADER_MAX_INPUT_TEXTURES];
	int samplerUniformLocations[SHADER_MAX_INPUT_TEXTURES];
	ShaderProgram shader;
};

extern struct CompiledShaderCache* get_compiled_shader(struct ShaderPass* shaderPass, struct RASPITEX_Texture ** inputs);

#ifdef __cplusplus
}
#endif

#endif
