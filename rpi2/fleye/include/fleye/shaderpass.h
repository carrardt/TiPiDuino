#ifndef __fleye_ShaderPass_H_
#define __fleye_ShaderPass_H_

#include "fleye/config.h"
#include "fleye/compiledshadercache.h"
#include "fleye/textureinput.h"

#include <vector>

struct GLTexture;
struct FrameBufferObject;
typedef void(*GLRenderFunctionT)(CompiledShaderCache*,int) ;

struct ShaderPass
{
	std::vector<TextureInput> inputs;
	std::string vertexSource;
	std::string fragmentSourceWithoutTextures;
	std::vector<CompiledShaderCache> shaderCahe;
	std::vector<FrameBufferObject*> fboPool;

	GLTexture* finalTexture;	
	GLRenderFunctionT gl_draw; //void(*gl_draw)(struct CompiledShaderCache*,int);
	int numberOfPasses;
	
	inline ShaderPass() : gl_draw(0), numberOfPasses(0), finalTexture(0) {}
};

#endif
