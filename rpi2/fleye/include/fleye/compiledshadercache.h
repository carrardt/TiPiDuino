#ifndef fleye_CompiledShaderCache_H_
#define fleye_CompiledShaderCache_H_

#include "fleye/shaderprogram.h"

struct GLTexture;
struct ShaderPass;

struct CompiledShaderCache
{
	std::vector<int> textureTargets;
	std::vector<int> samplerUniformLocations;
	ShaderProgram shader;
};

CompiledShaderCache* get_compiled_shader(ShaderPass* shaderPass, int passCount);

#endif
