#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "fleye/compiledshadercache.h"
#include "fleye/shaderprogram.h"
#include "fleye/shaderpass.h"
#include "fleye/texture.h"

struct CompiledShaderCache* get_compiled_shader(struct ShaderPass* shaderPass,struct RASPITEX_Texture** inputs)
{
	const char* image_external_pragma = "";
	char textureUniformProlog[1024]={'\0',};
	char* fragmentSource = 0;
	int i,rc;
	
	for(i=0;i<shaderPass->compileCacheSize;i++)
	{
		int j,found=1;
		for(j=0;j<shaderPass->nInputs && found;j++)
		{
			if( shaderPass->shaderCahe[i].textureTargets[j] != inputs[j]->target ) found=0;
		}
		if(found)
		{
			//printf("found precompiled shader %d\n",i);
			return & shaderPass->shaderCahe[i];
		}
	}
	if(shaderPass->compileCacheSize>=SHADER_COMPILE_CACHE_SIZE)
	{
		fprintf(stderr,"Shader cache is full");
		return 0;
	}

	struct CompiledShaderCache* compiledShader = & shaderPass->shaderCahe[ shaderPass->compileCacheSize ++ ];
	for(i=0;i<shaderPass->nInputs;i++)
	{
		const char* samplerType = 0;
		//const char* texlookup = "texture2D";
		char uniformDeclare[128];
		switch( inputs[i]->target )
		{
			case GL_TEXTURE_2D:
				samplerType = "sampler2D";
				break;
			case GL_TEXTURE_EXTERNAL_OES:
				samplerType = "samplerExternalOES";
				image_external_pragma = "#extension GL_OES_EGL_image_external : require\n";
				break;
			default:
				fprintf(stderr,"unhandled texture target");
				return 0;
				break;
		}
		compiledShader->textureTargets[i]=inputs[i]->target;
		sprintf(uniformDeclare,"uniform %s %s;\n",samplerType,shaderPass->inputs[i].uniformName);
		strcat(textureUniformProlog,uniformDeclare);
	}
	fragmentSource = malloc( strlen(image_external_pragma) + strlen(shaderPass->fragmentSourceWithoutTextures) + strlen(textureUniformProlog) + 8 );
	sprintf(fragmentSource,"%s\n%s\n%s\n",image_external_pragma,textureUniformProlog,shaderPass->fragmentSourceWithoutTextures);

	// printf("%s FS:\n%s",shaderPass->finalTexture->name,fragmentSource);
	// compile assembled final shader program
	rc = create_image_shader( & compiledShader->shader, shaderPass->vertexSource, fragmentSource );
	free(fragmentSource);
	if( rc != 0)
	{
		fprintf(stderr,"failed to build shader");
		return 0;
	}

	for(i=0;i<shaderPass->nInputs;i++)
	{
		compiledShader->samplerUniformLocations[i] = glGetUniformLocation(compiledShader->shader.program, shaderPass->inputs[i].uniformName);
		// printf("sampler uniform '%s' -> location %d\n",shaderPass->inputs[i].uniformName,compiledShader->samplerUniformLocations[i]);
	}

	return compiledShader;
}
