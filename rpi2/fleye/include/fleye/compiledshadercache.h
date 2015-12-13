#ifndef fleye_CompiledShaderCache_H_
#define fleye_CompiledShaderCache_H_

#include "fleye/shaderprogram.h"

struct RASPITEX_Texture;
struct ShaderPass;

struct CompiledShaderCache
{
	int textureTargets[SHADER_MAX_INPUT_TEXTURES];
	int samplerUniformLocations[SHADER_MAX_INPUT_TEXTURES];
	ShaderProgram shader;
};

#endif
