#ifndef fleye_CompiledShader_H_
#define fleye_CompiledShader_H_

#include "fleye/shaderprogram.h"

struct GLTexture;
struct ShaderPass;

struct CompiledShader
{
	std::vector<int> textureTargets;
	std::vector<int> samplerUniformLocations;
	ShaderProgram shader;
};

CompiledShader* get_compiled_shader(ShaderPass* shaderPass, int passCount);

#endif
