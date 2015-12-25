#include "fleye/compiledshader.h"
#include "fleye/shaderprogram.h"
#include "fleye/shaderpass.h"
#include "fleye/texture.h"

#include <assert.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <string>
#include <iostream>

CompiledShader* get_compiled_shader(ShaderPass* shaderPass, int passIteration)
{
	std::string image_external_pragma;
	std::string uniformDeclare;
	int i,rc;
	
	//std::cout<<shaderPass<<" "<<shaderPass->inputs.back().texPool.size()<<"\n";
	int nTextureInputs = shaderPass->inputs.size();
	std::vector<int> texTargets(nTextureInputs,GL_NONE);
	for(int i=0;i<nTextureInputs;i++)
	{ 
		const TextureInput & texinput = shaderPass->inputs[i] ;
		int texPoolSize = texinput.texPool.size();
		if( texPoolSize > 0 )
		{
			
			GLTexture* tex = texinput.texPool[ passIteration % texPoolSize ];
			if( tex != 0 ) { texTargets[i] = tex->target ; }
			else { std::cerr<<"Null texture pointer : input "<<i<<", pass "<<passIteration<<"\n"; }
		}
		else { std::cerr<<"no texture for input "<<i<<"\n";  }
	}

	for( CompiledShader& cs :  shaderPass->shaderCahe )
	{
		bool match = true;
		assert( nTextureInputs == cs.textureTargets.size() );
		for(int i=0;i<nTextureInputs && match;i++)
		{ 
			if(cs.textureTargets[i] != texTargets[i] ) match = false;
		}
		if( match ) { return &cs; }
	}
	
	CompiledShader compiledShader;
	compiledShader.textureTargets = texTargets ;
	for(int i=0;i<nTextureInputs;i++)
	{
		std::string samplerType;
		switch( compiledShader.textureTargets[i] )
		{
			case GL_TEXTURE_2D:
				samplerType = "sampler2D";
				break;
			case GL_TEXTURE_EXTERNAL_OES:
				samplerType = "samplerExternalOES";
				image_external_pragma = "#extension GL_OES_EGL_image_external : require\n";
				break;
			default:
				std::cerr<<"unhandled texture target "<<compiledShader.textureTargets[i];
				return 0;
		}
		uniformDeclare += "uniform " + samplerType + " " + shaderPass->inputs[i].uniformName + ";\n" ;
	}

	std::string fragmentSource = image_external_pragma + uniformDeclare + shaderPass->fragmentSourceWithoutTextures;
	// printf("%s FS:\n%s",shaderPass->finalTexture->name,fragmentSource);
	// compile assembled final shader program
	
	rc = create_image_shader( & compiledShader.shader, shaderPass->vertexSource.c_str(), fragmentSource.c_str() );
	if( rc != 0)
	{
		std::cerr<<"failed to build shader\n";
		return 0;
	}

	for( auto texInput : shaderPass->inputs )
	{
		compiledShader.samplerUniformLocations.push_back( glGetUniformLocation(compiledShader.shader.program, texInput.uniformName.c_str() ) );
		// printf("sampler uniform '%s' -> location %d\n",shaderPass->inputs[i].uniformName,compiledShader->samplerUniformLocations[i]);
	}

	shaderPass->shaderCahe.push_back(compiledShader);
	return & shaderPass->shaderCahe.back();
}
